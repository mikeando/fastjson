single :
	saru-run-test single $$( ls -t *.cpp | head -1 )
suite :
	saru-run-test suite .
