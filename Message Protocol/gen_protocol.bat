flatc --cpp -o ./schema/ ./schema/game_message.fbs --gen-object-api
flatc -n -o ./schema/ ./schema/game_message.fbs --gen-onefile
