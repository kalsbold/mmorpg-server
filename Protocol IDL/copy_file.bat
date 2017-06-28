xcopy /y generated\protocol_cs_generated.h ..\Game
xcopy /y generated\protocol_ss_generated.h ..\Game
rd /s/q ..\..\MMOUnityClient\Assets\Scripts\Network\Protocol
xcopy /y/s/e generated\ProtocolCS ..\..\MMOUnityClient\Assets\Scripts\Network\Protocol
