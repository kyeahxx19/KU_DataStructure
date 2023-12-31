#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_YEAR_DURATION	10	// 기간
#define LINEAR_SEARCH 0
#define BINARY_SEARCH 1

// 구조체 선언
typedef struct {
	char	name[20];		// 이름
	char	sex;			// 성별 'M' or 'F'
	int		freq[MAX_YEAR_DURATION]; // 연도별 빈도
} tName;
 
typedef struct {
	int		len;		// 배열에 저장된 이름의 수
	int		capacity;	// 배열의 용량 (배열에 저장 가능한 이름의 수)
	tName	*data;		// 이름 배열의 포인터
} tNames;

////////////////////////////////////////////////////////////////////////////////
// 함수 원형 선언(declaration)

// 연도별 입력 파일을 읽어 이름 정보(이름, 성별, 빈도)를 이름 구조체에 저장
// 이미 구조체에 존재하는(저장된) 이름은 해당 연도의 빈도만 저장
// 새로 등장한 이름은 구조체에 추가
// 주의사항: 동일 이름이 남/여 각각 사용될 수 있으므로, 이름과 성별을 구별해야 함
// names->capacity는 1000으로부터 시작하여 1000씩 증가 (1000, 2000, 3000, ...)
// start_year : 시작 연도 (2009)
// 선형탐색(linear search) 버전
void load_names_lsearch(FILE *fp, int start_year, tNames *names);

// 이진탐색(binary search) 버전
// bsearch 함수 이용; qsort 함수를 이용하여 이름 구조체의 정렬을 유지해야 함
void load_names_bsearch(FILE *fp, int start_year, tNames *names);

// 구조체 배열을 화면에 출력
void print_names( tNames *names, int num_year);

// qsort, bsearch를 위한 비교 함수
// 정렬 기준 : 이름(1순위), 성별(2순위)
int compare( const void *n1, const void *n2);

////////////////////////////////////////////////////////////////////////////////
// 함수 정의 (definition)

// 이름 구조체를 초기화
// len를 0으로, capacity를 1000으로 초기화
// return : 구조체 포인터
tNames *create_names(void)
{
	tNames *pnames = (tNames *)malloc( sizeof(tNames));
	
	pnames->len = 0;
	pnames->capacity = 1000;
	pnames->data = (tName *)malloc(pnames->capacity * sizeof(tName));

	return pnames;
}

// 이름 구조체에 할당된 메모리를 해제
void destroy_names(tNames *pnames)
{
	free(pnames->data);
	pnames->len = 0;
	pnames->capacity = 0;

	free(pnames);
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	tNames *names;
	int option;
	FILE *fp;
	
	if (argc != 3)
	{
		fprintf( stderr, "Usage: %s option FILE\n\n", argv[0]);
		fprintf( stderr, "option\n\t-l\n\t\twith linear search\n\t-b\n\t\twith binary search\n");
		return 1;
	}
	
	if (strcmp( argv[1], "-l") == 0) option = LINEAR_SEARCH;
	else if (strcmp( argv[1], "-b") == 0) option = BINARY_SEARCH;
	else {
		fprintf( stderr, "unknown option : %s\n", argv[1]);
		return 1;
	}
	
	// 이름 구조체 초기화
	names = create_names();

	if ((fp = fopen( argv[2], "r")) == NULL) 
	{
		fprintf( stderr, "cannot open file : %s\n", argv[2]);
		return 1;
	}

	if (option == LINEAR_SEARCH)
	{
		// 연도별 입력 파일(이름 정보)을 구조체에 저장
		// 선형탐색 모드
		load_names_lsearch( fp, 2009, names);
	}
	else //(option == BINARY_SEARCH)
	{
		// 이진탐색 모드
		load_names_bsearch( fp, 2009, names);
	}

	// 정렬 (이름순 (이름이 같은 경우 성별순))
	qsort( names->data, names->len, sizeof(tName), compare);

	fclose( fp);
		
	// 이름 구조체를 화면에 출력
	print_names( names, MAX_YEAR_DURATION);

	// 이름 구조체 해제
	destroy_names( names);
	
	return 0;
}

void load_names_lsearch(FILE *fp, int start_year, tNames *names)
{

	int year, freq;
	char sex, name[20];
	tName *iptr = names->data, *jptr = NULL, *bptr = names->data;

	if (fp != NULL)
	{

		while (!feof(fp))
		{
			freq = 0;

			fscanf(fp, "%d %s %c %d", &year, name, &sex, &freq);

			if (year == start_year)
			{
				strcpy(iptr->name, name);
				iptr->sex = sex;
				iptr->freq[year - start_year] += freq;

				names->len++;

				if (names->capacity == names->len)
				{
					names->capacity += 1000;
					names->data = (tName *)realloc(names->data, names->capacity * sizeof(tName));
				}

				iptr++;
			}

			else
			{

				jptr = names->data;

				while (year != start_year && jptr <= iptr)
				{

					if (!strcmp(name, jptr->name) && sex == jptr->sex)
					{
						jptr->freq[year - start_year] += freq;
						break;
					}

					else if (jptr == iptr)
					{
						strcpy(iptr->name, name);
						iptr->sex = sex;
						iptr->freq[year - start_year] += freq;

						names->len++;
						if (names->capacity == names->len)
						{
							names->capacity += 1000;
							names->data = (tName *)realloc(names->data, names->capacity * sizeof(tName));
						}

						iptr++;
						break;
					}

					else
					{
						jptr++;
					}
				}
			}
		}
	}
}

// 이진탐색(binary search) 버전
// bsearch 함수 이용; qsort 함수를 이용하여 이름 구조체의 정렬을 유지해야 함
void load_names_bsearch(FILE *fp, int start_year, tNames *names)
{

	int year, f, size, flag = start_year + 1;
	char sex, name[20];
	tName *iptr = names->data, *find = NULL, *key = NULL;

	if (fp != NULL)
	{
		while (!feof(fp))
		{
			fscanf(fp, "%d\t%s\t%c\t%d", &year, name, &sex, &f);

			if (year == flag)
			{
				qsort(names->data, names->len, sizeof(tName), compare);
				size = names->len;
				flag++;
			}
			
			if (year == start_year)
			{
				strcpy(iptr->name, name);
				iptr->sex = sex;
				for (int i = 0; i < 10; i++)
					iptr->freq[i] = 0;
				iptr->freq[year - start_year] = f;
				
				names->len++;

				if (names->capacity == names->len)
				{
					names->capacity += 1000;
					names->data = (tName *)realloc(names->data, names->capacity * sizeof(tName));
				}
				
				iptr++;
			}

			else
			{
				strcpy(key->name, name);
				key->sex = sex;

				if ((find = (tName *)bsearch(key, names->data, size, sizeof(tName), compare)) == NULL)
				{
					strcpy(iptr->name, name);
					iptr->sex = sex;
					for (int i = 0; i < 10; i++)
						iptr->freq[i] = 0;
					find->freq[year - start_year] = f;

					names->len++;

					if (names->capacity == names->len)
					{
						names->capacity += 1000;
						names->data = (tName *)realloc(names->data, names->capacity * sizeof(tName));
					}

					iptr++;
				}
				else
				{
					find->freq[year - start_year] = f;
				}
			}
		}
	}
}

// 구조체 배열을 화면에 출력
void print_names( tNames *names, int num_year) 
{
	int loc = 0;
	tName *pLoc = names->data;

	while (names->len > loc++) {

		printf("%s\t%c", pLoc->name, pLoc->sex);

		for (int i=0; i<10; i++) {
			printf("\t%d", pLoc->freq[i]);
		}
		printf("\n");
		
		pLoc++;
	}
}

// qsort, bsearch를 위한 비교 함수
// 정렬 기준 : 이름(1순위), 성별(2순위)
int compare( const void *n1, const void *n2) 
{
	const tName *m1 = (const tName *)n1;
	const tName *m2 = (const tName *)n2;

	if (strcmp(m1->name, m2->name) > 0)
	{
		return 1;
	}
	
	else if (strcmp(m1->name, m2->name) < 0)
	{
		return -1;
	}
	
	else 
	{
		if (m1->sex > m2->sex)
		{
			return 1;
		}
		else if (m1->sex < m2->sex)
		{
			return -1;
		}
		else return 0;
	}
}