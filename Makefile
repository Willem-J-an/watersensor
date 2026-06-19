all: compile upload monitor

compile:
	arduino-cli compile -e
upload:
	arduino-cli upload --upload-property upload.speed=115200
monitor:
	arduino-cli monitor -c baudrate=115200
list:
	arduino-cli board list
upload_z2m: ## upload z2m def.js spec
	smbclient //192.168.1.210/share -U willem -c "put def.js"
	ssh -tt willem@192.168.1.210 "sudo docker cp ~/share/def.js z2m:/app/data/external_converters/watersensor.js && sudo systemctl restart z2m"
debug: ## needs the code
	cat stack.txt | grep -o '0x[0-9a-fA-F]\+' | xargs /Users/w13635/Library/Arduino15/packages/esp32/tools/esp-rv32/2511/bin/riscv32-esp-elf-addr2line -pfiaC -e ./build/esp32.esp32.esp32c6/tds.ino.elf | grep -v '?? ??:0'
