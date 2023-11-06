all:
	python3 generate_neo_test_cases.py 4 4 4 > ../neo_resnet50_1.c
	python3 generate_neo_test_cases.py 4 4 4 > ../neo_resnet50_3.c
	python3 generate_neo_test_cases.py 3136 576 64    > ../neo_resnet50_2.c
	python3 generate_neo_test_cases.py 256 768  768   > ../neo_bert_1.c
	python3 generate_neo_test_cases.py 256 3072 768   > ../neo_bert_2.c
	python3 generate_neo_test_cases.py 256 768 3072   > ../neo_bert_3.c
	python3 generate_neo_test_cases.py 512 1024 1024  > ../neo_dlrm_1.c
	python3 generate_neo_test_cases.py 512 1024 64    > ../neo_dlrm_2.c
	python3 generate_neo_test_cases.py 512 2048 2048  > ../neo_dlrm_3.c
	python3 generate_neo_test_cases.py 1  3072 768   > ../neo_bert_2_n1.c
	python3 generate_neo_test_cases.py 4  3072 768   > ../neo_bert_2_n4.c
	python3 generate_neo_test_cases.py 8  3072 768   > ../neo_bert_2_n8.c
	python3 generate_neo_test_cases.py 16 3072 768   > ../neo_bert_2_n16.c
	python3 generate_neo_test_cases.py 32 3072 768   > ../neo_bert_2_n32.c
	python3 generate_neo_test_cases.py 64 3072 768   > ../neo_bert_2_n64.c
	python3 generate_neo_test_cases.py 64 64 64   > ../neo_test_1.c
