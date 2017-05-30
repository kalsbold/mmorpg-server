flatc --cpp -o ./ ./protocol.fbs --gen-object-api --scoped-enums
flatc --csharp -o ./ ./protocol.fbs
