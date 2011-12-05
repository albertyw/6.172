echo './cache-scratch 1 1000 8 1000000	'
./cache-scratch 1 1000 8 1000000	
echo './cache-scratch 12 1000 8 1000000'
./cache-scratch 12 1000 8 1000000
echo ''
echo ''
echo './cache-thrash 1 1000 8 1000000'
./cache-thrash 1 1000 8 1000000
echo './cache-thrash 12 1000 8 1000000'
./cache-thrash 12 1000 8 1000000
echo ''
echo ''
echo './growvector 1 100000 8'
./growvector 1 100000 8
echo './growvector 12 100000 8'
./growvector 12 100000 8
echo ''
echo ''
echo './larson 10 7 16 1000 10000 6172 12'
./larson 10 7 16 1000 10000 6172 12
echo './larson 10 7 8 1000 10000 6172 12'
./larson 10 7 8 1000 10000 6172 12
echo ''
echo ''
echo './linux-scalability 8 10000000 1'
./linux-scalability 8 10000000 1
echo './linux-scalability 8 10000000 12'
./linux-scalability 8 10000000 12
echo './linux-scalability 999 10000000 12'
./linux-scalability 999 10000000 12
