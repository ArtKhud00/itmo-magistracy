.DEFAULT_GOAL := help

.PHONY:
SHELL=/bin/bash
SHELLOPTS:=$(if $(SHELLOPTS),$(SHELLOPTS):)pipefail:errexit
ENVIRONMENT ?= test

export

include ./.__internal/helper.mk

.PHONY: help
help: ## Display this help
	@IFS=$$'\n'; for line in `grep -h -E '^[a-zA-Z_#-]+:?.*?## .*$$' $(MAKEFILE_LIST)`; do if [ "$${line:0:2}" = "##" ]; then \
	echo $$line | awk 'BEGIN {FS = "## "}; {printf "\n\033[33m%s\033[0m\n", $$2}'; else \
	echo $$line | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'; fi; \
	done; unset IFS;

deploy: ## Deploy application to specifi environment
	ansible-playbook -i inventory/default/ -i inventory/$(ENVIRONMENT)/ product-deploy.yaml -e "privilege_user=$(USER)"

rollback: ## Rollback application to specifi environment
	ansible-playbook -i inventory/default/ -i inventory/$(ENVIRONMENT)/ product-rollback.yaml -e "privilege_user=$(USER)" 