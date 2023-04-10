mkdir -p deploy/client
mkdir -p deploy/dashboard
cp -r build/SideAssist.Client.Qt/Release/* deploy/client/
cp -r SideAssist.Dashboard.Flutter/build/windows/runner/Release/* deploy/dashboard/
