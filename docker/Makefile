exec:
	docker exec -it docker-slurmmaster-1 bash

start: create
	docker-compose start

create:
	docker-compose create

stop:
	docker-compose stop

clean:
	docker-compose rm

deps:
	sudo apt-get -y update
	sudo apt-get -y install cmake libpoco-dev nano

deps_all:
	srun -N 3 make deps

help:
	@echo "make <option>:"
	@echo ""
	@echo "exec   : Execute bash in slurm master"
	@echo "create : Create images"
	@echo "stop   : Stop containers"
	@echo "clean  : Delete containers"
	@echo "deps   : Install dependencies(inside docker)"
