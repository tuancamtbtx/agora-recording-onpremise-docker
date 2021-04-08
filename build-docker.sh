# replace "/" by "-", vì docker tag không cho phép dấu "/"
ORIGINAL_BRAND_NAME=$(git symbolic-ref --short HEAD)
BRAND_NAME=${ORIGINAL_BRAND_NAME//[\/]/-}
DOCKER_IMAGE=name_image
DOCKER_REGISTRY=registry
#-----------------------------------------------------------------------
docker build . -t ${DOCKER_REGISTRY}/${DOCKER_IMAGE}:${BRAND_NAME}
docker push ${DOCKER_REGISTRY}/${DOCKER_IMAGE}:${BRAND_NAME}
