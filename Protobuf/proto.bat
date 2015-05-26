protoc.exe -I=. --cpp_out=. MyPacket.proto
copy MyPacket.proto ProtoGen
cd "ProtoGen"
protogen -i:"MyPacket.proto" -o:"..\MyPacket.cs" -d
del "./MyPacket.proto"