use lambda_http::{lambda, IntoResponse, Request, RequestExt};
use lambda_runtime::{error::HandlerError, Context, start};
use serde::{Deserialize, Serialize};
use serde::export::PhantomData;

#[derive(Serialize, Deserialize, Clone)]
struct CustomEvent {
    #[serde(rename = "name")]
    name: String,
}

#[derive(Serialize, Deserialize, Clone)]
struct CustomOutput {
    message: String,
}

fn main() {
    start(my_handler, None);
}

fn my_handler(e: CustomEvent, _ctx: Context) -> Result<CustomOutput, HandlerError> {
    let name = e.name;
    let message = format!(
        "hello {}",
        if name.is_empty() { "stranger".to_string() } else { name }
    );

    Ok(CustomOutput { message })
}
