clean:
		@rm -rf server/.pio server/.vscode client/__pycache__/ client/lib/__pycache__/ client/src/__pycache__/ client/lib/communication/__pycache__ client/lib/gui/__pycache__/ client/lib/session/__pycache__

client:
		@python3 -m client.src.client

server:
		cd server && pio run -t upload

.PHONY: clean server client 