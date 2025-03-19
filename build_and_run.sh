make dev
./out/main "$1"
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Compilation errored, canceling program execution..."
else 
    ./emulator -i a.out -o __stdout__
fi
exit $retVal