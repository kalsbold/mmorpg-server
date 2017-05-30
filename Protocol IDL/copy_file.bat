xcopy /y .\protocol_generated.h ..\Game
rd /s/q ..\..\MMOUnityClient\Assets\Scripts\Network\Protocol
xcopy /y/s/e .\protocol ..\..\MMOUnityClient\Assets\Scripts\Network\Protocol
