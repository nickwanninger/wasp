#!/usr/bin/env bash
set -e

EXEC_NAME=fib_dataflow_serverless
AWS_FUNCTION_NAME=wasp-fib-dataflow

IMAGE_NAME=wasp/serverless-builder
CONTAINER_NAME=wasp-builder-dummy
LOCAL_TARGET_DIR=target/aws-linux-musl
EXEC_PATH=$LOCAL_TARGET_DIR/$EXEC_NAME

LAMBDA_ZIP_NAME=lambda.zip
LAMBDA_ZIP_PATH=$LOCAL_TARGET_DIR/$LAMBDA_ZIP_NAME
LAMBDA_EXEC_NAME=bootstrap  # required by AWS Lambda runtime
LAMBDA_EXEC_PATH=$LOCAL_TARGET_DIR/$LAMBDA_EXEC_NAME


function finish {
	# remove builder container either way
	docker rm -fi $CONTAINER_NAME
}
trap finish EXIT


#
# cross compile with musl within docker (fixes issues with deps)
#
rm -rf $LOCAL_TARGET_DIR
mkdir -p $LOCAL_TARGET_DIR
docker build --tag $IMAGE_NAME -f build.Dockerfile .
docker create --name $CONTAINER_NAME $IMAGE_NAME /bin/false
docker cp $CONTAINER_NAME:/app/target/x86_64-unknown-linux-musl/release/$EXEC_NAME $LOCAL_TARGET_DIR/$EXEC_NAME
docker rm -f $CONTAINER_NAME


#
# zip up the lambda deployment
#
rm -f $LAMBDA_ZIP_PATH
cp $EXEC_PATH $LAMBDA_EXEC_PATH
zip --junk-paths $LAMBDA_ZIP_PATH $LAMBDA_EXEC_PATH
rm $LAMBDA_EXEC_PATH


#
# create or update function on AWS
#
EXIT_CODE=0
aws lambda get-function --function-name $AWS_FUNCTION_NAME > /dev/null 2>&1 || EXIT_CODE=$?
if [ $EXIT_CODE -eq 0 ]; then
	# function exists, update
	echo "updating function '$AWS_FUNCTION_NAME'..."

	aws lambda update-function-code \
		--function-name $AWS_FUNCTION_NAME \
		--zip-file fileb://$LAMBDA_ZIP_PATH

else
	# function does not exist, create
	echo "creating function '$AWS_FUNCTION_NAME'..."

	aws lambda create-function --function-name $AWS_FUNCTION_NAME \
		--handler wasp-fib-handler \
		--zip-file fileb://$LAMBDA_ZIP_PATH \
		--runtime provided \
		--role arn:aws:iam::225760913492:role/wasp-lambda-execute \
		--environment Variables={RUST_BACKTRACE=1} \
		--tracing-config Mode=Active
fi
