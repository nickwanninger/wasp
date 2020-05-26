use std::convert::TryInto;
use std::io::Bytes;
use futures::try_join;
use lambda::{INVOCATION_CTX, handler_fn, Handler, LambdaCtx};
use serde::{Deserialize, Serialize};
use serde::export::PhantomData;
use rusoto_core::{Region, RusotoError};
use rusoto_lambda::{InvocationRequest, InvokeError, Lambda, LambdaClient, ListFunctionsRequest};

type Error = Box<dyn std::error::Error + Send + Sync + 'static>;

#[derive(Serialize, Deserialize, Clone)]
struct MyInput {
    #[serde(rename = "x")]
    input: u32,
}

#[derive(Serialize, Deserialize, Clone)]
struct MyOutput {
    output: u32,
}

#[tokio::main]
async fn main() -> Result<(), Error> {
    println!("wasp-serverless");
    let func = handler_fn(my_handler);
    lambda::run(func).await?;
    Ok(())
}

async fn my_handler(e: MyInput) -> Result<MyOutput, Error> {
    let n = e.input;
    if n == 0 || n == 1 {
        return Ok(MyOutput { output: n });
    }

    let client = LambdaClient::new(Region::UsEast1);
    let subcall = RecurseContext { client };
    let fib1_fut = subcall.invoke(n - 1);
    let fib2_fut = subcall.invoke(n - 2);
    let (fib1, fib2) = try_join!(fib1_fut, fib2_fut)?;

    let result = fib1.output + fib2.output;
    Ok(MyOutput { output: result })
}


struct RecurseContext {
    client: LambdaClient,
}


impl RecurseContext {
    async fn invoke<'a>(&self, input: u32) -> Result<MyOutput, Error> {
        let ctx = INVOCATION_CTX.with(|x: &LambdaCtx| x.clone());

        let payload = MyInput { input };
        let payload_json = serde_json::to_string(&payload)?;
        let payload_bytes = payload_json.into_bytes();

        let function_name = ctx.invoked_function_arn;
        println!("my arn = '{}'", function_name);

        let request = InvocationRequest {
            function_name,
            invocation_type: Some("RequestResponse".to_string()),
            payload: Some(payload_bytes.into()),
            ..Default::default()
        };

        let response = self.client.invoke(request).await;
        let output_payload = response?.payload.unwrap();
        let output = serde_json::from_slice::<MyOutput>(&output_payload)?;

        Ok(output)
    }
}