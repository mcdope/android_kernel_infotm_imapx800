#!/system/bin/sh

./system/bin/ipc_test --func=sync --keystr=sync1 --role=owner --count=20000 &
./system/bin/ipc_test --func=sync --keystr=sync1 --role=slave --count=20000 &

./system/bin/ipc_test --func=sync --keystr=sync2 --role=slave --count=20000 &
./system/bin/ipc_test --func=sync --keystr=sync2 --role=owner --count=20000 &

./system/bin/ipc_test --func=sync --keystr=sync3 --role=slave --count=20000 &
./system/bin/ipc_test --func=sync --keystr=sync3 --role=owner --count=20000 &

./system/bin/ipc_test --func=sync --keystr=sync4 --role=owner --count=20000 &
./system/bin/ipc_test --func=sync --keystr=sync4 --role=slave --count=20000 &


./system/bin/ipc_test --func=shm --keystr=shm1 --role=slave --count=20000 &
./system/bin/ipc_test --func=shm --keystr=shm1 --role=owner --count=20000 &

./system/bin/ipc_test --func=shm --keystr=shm2 --role=owner --count=20000 &
./system/bin/ipc_test --func=shm --keystr=shm2 --role=slave --count=20000 &

./system/bin/ipc_test --func=shm --keystr=shm3 --role=owner --count=20000 &
./system/bin/ipc_test --func=shm --keystr=shm3 --role=slave --count=20000 &

./system/bin/ipc_test --func=shm --keystr=shm4 --role=slave --count=20000 &
./system/bin/ipc_test --func=shm --keystr=shm4 --role=owner --count=20000 &


./system/bin/ipc_test --func=pipe --keystr=pipe1 --role=slave --count=20000 &
./system/bin/ipc_test --func=pipe --keystr=pipe1 --role=owner --count=20000 &

./system/bin/ipc_test --func=pipe --keystr=pipe2 --role=slave --count=20000 &
./system/bin/ipc_test --func=pipe --keystr=pipe2 --role=owner --count=20000 &

./system/bin/ipc_test --func=pipe --keystr=pipe3 --role=slave --count=20000 &
./system/bin/ipc_test --func=pipe --keystr=pipe3 --role=owner --count=20000 &

./system/bin/ipc_test --func=pipe --keystr=pipe4 --role=owner --count=20000 &
./system/bin/ipc_test --func=pipe --keystr=pipe4 --role=slave --count=20000 &
