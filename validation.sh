echo 'python validate.py ./cache-scratch-validate 1 1000 8 1000000	'
python validate.py ./cache-scratch-validate 1 1000 8 1000000
echo ''
echo ''
echo 'python validate.py ./cache-thrash-validate 1 1000 8 1000000'
python validate.py ./cache-thrash-validate 1 1000 8 1000000
echo ''
echo ''
echo 'python validate.py ./growvector-validate 1 100000 8'
python validate.py ./growvector-validate 1 100000 8
echo ''
echo ''
echo 'python validate.py ./larson-validate 10 7 16 1000 10000 6172 12'
python validate.py ./larson-validate 10 7 16 1000 10000 6172 12
echo ''
echo ''
echo 'python validate.py ./linux-scalability-validate 8 10000000 1'
python validate.py ./linux-scalability-validate 8 10000000 1
