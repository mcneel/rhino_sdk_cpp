#!/usr/bin/env bash
#First command-line arg should be full path to app bundle, e.g.: ./make_rhino_debuggable.sh "/Applications/Rhino 9.app"
#Shell script to allow Rhino to be debugged for Rhino for Mac SDK plugins.

APP="$1"

#Accept a path inside the bundle too, such as Rhino 9.app/Contents/MacOS/Rhinoceros.
while [ -n "$APP" ] && [ "$APP" != "/" ] && [ "$APP" != "." ] && [[ "$APP" != *.app ]]; do
	APP=$(dirname "$APP")
done

if [[ "$APP" != *.app ]] || [ ! -d "$APP" ]; then
	echo "Usage: $0 /path/to/Rhino.app"
	exit 1
fi

TEMP_ENTITLEMENTS="$HOME/rhino.entitlements"

#Get rid of any previous detritus
rm -f "$TEMP_ENTITLEMENTS" "$TEMP_ENTITLEMENTS.bak"

#Extract the entitlements for app bundle to an XML file
codesign --display --xml --entitlements "$TEMP_ENTITLEMENTS" "$APP"

if grep -wq "get-task-allow" "$TEMP_ENTITLEMENTS"; then
	echo "Rhino is already debuggable"
else
	#Replace a portion of the XML to inject the get-task-allow entitlement
	sed -i.bak "s%<key>com.apple.security.cs.allow-jit</key><true/>%<key>com.apple.security.cs.allow-jit</key><true/><key>com.apple.security.get-task-allow</key><true/>%" "$TEMP_ENTITLEMENTS"

	#cat "$TEMP_ENTITLEMENTS"

	#Inject the modified entitlements into the app bundle.
	codesign -s - --deep --force --options=runtime --entitlements "$TEMP_ENTITLEMENTS" "$APP"

	#Clear the quarantine a never-opened download still carries, or macOS will not run the re-signed copy from where it is.
	xattr -dr com.apple.quarantine "$APP" 2>/dev/null

	echo "Rhino is now debuggable"
fi

#Clean up the temporary working files
rm -f "$TEMP_ENTITLEMENTS" "$TEMP_ENTITLEMENTS.bak"
