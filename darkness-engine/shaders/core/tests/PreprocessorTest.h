#define A
#define B
#define C
#define D
#define E
#define F

#define TestPreprocessor

#ifdef TestPreprocessor
float DEFINE_TEST_SUCCESS;
#endif

#if defined(TestPreprocessor)
float IF_DEFINED_TEST_SUCCESS;
#endif

#ifdef Blah
float UNDEFINED_TEST_FAILED;
#else
float ELSE_TEST_SUCCESS;
#endif

#undef TestPreprocessor

#ifdef TestPreprocessor
float UNDEF_TEST_FAILED;
#else
float UNDEF_TEST_SUCCEEDED;
#endif

#if 0
float EVALUATE_TEST_FAILED;
#else
float EVALUATE_TEST_SUCCEEDED;
#endif

#define Somedef
#define TestPreprocessor
#define TestEval
#define abc

#if defined(TestPreprocessor) && defined(TestEval)
float EVALUATE_COMPLEX_TEST_SUCCEEDED;
#else
float EVALUATE_COMPLEX_TEST_FAILED;
#endif

#ifdef DefineFromAnotherFile
float DEFINE_FROM_ANOTHER_FILE_TEST_SUCCEEDED;
#else
float DEFINE_FROM_ANOTHER_FILE_TEST_FAILED;
#endif
