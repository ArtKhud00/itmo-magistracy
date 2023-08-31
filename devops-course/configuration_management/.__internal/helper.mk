### Internal Helper for generate and operate dev environment
# ¯¯¯¯¯¯¯¯
.PHONY:
HELPER_ENV_MAKE_PATH:=$(abspath $(lastword $(MAKEFILE_LIST)))
HELPER_ENV_MAKE_DIR:=$(dir $(HELPER_ENV_MAKE_PATH))

SHELL=/bin/bash
SHELLOPTS:=$(if $(SHELLOPTS),$(SHELLOPTS):)pipefail:errexit

__USER?=$(shell whoami)

__PORT_NUMBER_PART:=$(shell echo "${__USER}" | sed 's/[^0-9]*//g')
__PORT_NUMBER_PART:=$(shell [[ "${__PORT_NUMBER_PART}" =~ ^[0-9]+$$ ]] && echo "${__PORT_NUMBER_PART}" || echo "00")

__IMAG_TAG_NAME:=dev_environment:ubi7

.PHONY: echo
echo:
	@echo ${__PORT_NUMBER_PART}

.PHONY: build-dev-image
build-dev-image:
	@docker build -t ${__IMAG_TAG_NAME} \
	-f ${HELPER_ENV_MAKE_DIR}docker/ubiDockerfile ${HELPER_ENV_MAKE_DIR}docker/.

.PHONY: up-env
up-env:
	@docker-compose -f ${HELPER_ENV_MAKE_DIR}docker/docker-compose.yml \
		-p ${__USER} \
		up -d ${SERVICE_PREFIX}_service_1_host ${SERVICE_PREFIX}_service_2_host

.PHONY: stop-env
stop-env: ## Stop all environmets
	@docker-compose -f ${HELPER_ENV_MAKE_DIR}docker/docker-compose.yml \
		-p ${__USER} \
		down

.PHONY: attach-env
attach-env:
	@docker-compose -f ${HELPER_ENV_MAKE_DIR}docker/docker-compose.yml \
		-p ${__USER} \
		exec ${SERVICE_NAME} /bin/bash -c "sudo -iu red${__PORT_NUMBER_PART}"

.PHONY: root-env
root-env:
	@docker-compose -f ${HELPER_ENV_MAKE_DIR}docker/docker-compose.yml \
		-p ${__USER} \
		exec ${SERVICE_NAME} /bin/bash

.PHONY: generate-test
generate-test:	## Up test environment
	$(MAKE) stop-env && \
	$(MAKE) SERVICE_PREFIX=test up-env

.PHONY: generate-production
generate-production: ## Up production environment
	$(MAKE) stop-env && \
	$(MAKE) SERVICE_PREFIX=production up-env

.PHONY: attach-production-service-1
attach-production-service-1: ## Attach to production service 1
	$(MAKE) SERVICE_NAME=production_service_1_host attach-env

.PHONY: attach-production-service-2
attach-production-service-2: ## Attach to production service 2
	$(MAKE) SERVICE_NAME=production_service_2_host attach-env

.PHONY: attach-test-service-1
attach-test-service-1: ## Attach to test service 1
	$(MAKE) SERVICE_NAME=test_service_1_host attach-env

.PHONY: attach-production-service-2
attach-test-service-2: ## Attach to test service 2
	$(MAKE) SERVICE_NAME=test_service_2_host attach-env