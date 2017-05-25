flatc --cpp -o ./ ./protocol.fbs --gen-object-api
flatc -n -o ./ ./protocol.fbs --gen-onefile
