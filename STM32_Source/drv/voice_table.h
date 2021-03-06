
enum SamplesNames
{
  SAMPLE_0 = 0,
  SAMPLE_1m,
  SAMPLE_1w,
  SAMPLE_2m,
  SAMPLE_2w,
  SAMPLE_3,
  SAMPLE_4,
  SAMPLE_5,
  SAMPLE_6,
  SAMPLE_7,
  SAMPLE_8,
  SAMPLE_9,
  
  SAMPLE_10,
  SAMPLE_11,
  SAMPLE_12,
  SAMPLE_13,
  SAMPLE_14,
  SAMPLE_15,
  SAMPLE_16,
  SAMPLE_17,
  SAMPLE_18,
  SAMPLE_19,
  
  SAMPLE_20,
  SAMPLE_30,
  SAMPLE_40,
  SAMPLE_50,
  
  SAMPLE_rovno,
    
  SAMPLE_chas,
  SAMPLE_chasov,
  SAMPLE_chasa,
    
  SAMPLE_minut,
  SAMPLE_minuta,
  SAMPLE_minuty,
  
  SAMPLE_bell,

  SAMPLE_minus,
  SAMPLE_gradus,
  SAMPLE_gradusa,
  SAMPLE_gradusov,
  
  SAMPLE_temperatura_out,
  SAMPLE_temperatura_in,
  
  SAMPLE_alarm1,
  SAMPLE_alarm2,
  SAMPLE_alarm3,  
  SAMPLE_alarm4,
  SAMPLE_alarm5,
  SAMPLE_alarm6,
  SAMPLE_alarm7,
  SAMPLE_alarm8,
  SAMPLE_alarm9,
  SAMPLE_alarm10,
  SAMPLE_alarm11,
  SAMPLE_alarm12,
  SAMPLE_alarm13,
  SAMPLE_alarm14,
  SAMPLE_alarm15,
  SAMPLE_alarm16,
  
  SAMPLE_void = 0xFF
};

static uint8_t hours_table[24][3] = 
  /*0*/ { {SAMPLE_0,    SAMPLE_void,    SAMPLE_chasov},
  /*1*/   {SAMPLE_1m,   SAMPLE_void,    SAMPLE_chas},
  /*2*/   {SAMPLE_2m,   SAMPLE_void,    SAMPLE_chasa},
  /*3*/   {SAMPLE_3,    SAMPLE_void,    SAMPLE_chasa},
  /*4*/   {SAMPLE_4,    SAMPLE_void,    SAMPLE_chasa},
  /*5*/   {SAMPLE_5,    SAMPLE_void,    SAMPLE_chasov},
  /*6*/   {SAMPLE_6,    SAMPLE_void,    SAMPLE_chasov},
  /*7*/   {SAMPLE_7,    SAMPLE_void,    SAMPLE_chasov},
  /*8*/   {SAMPLE_8,    SAMPLE_void,    SAMPLE_chasov},
  /*9*/   {SAMPLE_9,    SAMPLE_void,    SAMPLE_chasov},
  /*10*/  {SAMPLE_10,   SAMPLE_void,    SAMPLE_chasov},
  /*11*/  {SAMPLE_11,   SAMPLE_void,    SAMPLE_chasov},
  /*12*/  {SAMPLE_12,   SAMPLE_void,    SAMPLE_chasov},
  /*13*/  {SAMPLE_13,   SAMPLE_void,    SAMPLE_chasov},
  /*14*/  {SAMPLE_14,   SAMPLE_void,    SAMPLE_chasov},
  /*15*/  {SAMPLE_15,   SAMPLE_void,    SAMPLE_chasov},
  /*16*/  {SAMPLE_16,   SAMPLE_void,    SAMPLE_chasov},
  /*17*/  {SAMPLE_17,   SAMPLE_void,    SAMPLE_chasov},
  /*18*/  {SAMPLE_18,   SAMPLE_void,    SAMPLE_chasov},
  /*19*/  {SAMPLE_19,   SAMPLE_void,    SAMPLE_chasov},
  /*20*/  {SAMPLE_20,   SAMPLE_void,    SAMPLE_chasov},
  /*21*/  {SAMPLE_20,   SAMPLE_1m,      SAMPLE_chas},
  /*22*/  {SAMPLE_20,   SAMPLE_2m,      SAMPLE_chasa},
  /*23*/  {SAMPLE_20,   SAMPLE_3,       SAMPLE_chasa} };


static uint8_t min_table[60][3] = 
  /*0*/ { {SAMPLE_rovno,    SAMPLE_void,    SAMPLE_void},
  /*1*/   {SAMPLE_1w,       SAMPLE_void,    SAMPLE_minuta},
  /*2*/   {SAMPLE_2w,       SAMPLE_void,    SAMPLE_minuty},
  /*3*/   {SAMPLE_3,        SAMPLE_void,    SAMPLE_minuty},
  /*4*/   {SAMPLE_4,        SAMPLE_void,    SAMPLE_minuty},
  /*5*/   {SAMPLE_5,        SAMPLE_void,    SAMPLE_minut},
  /*6*/   {SAMPLE_6,        SAMPLE_void,    SAMPLE_minut},
  /*7*/   {SAMPLE_7,        SAMPLE_void,    SAMPLE_minut},
  /*8*/   {SAMPLE_8,        SAMPLE_void,    SAMPLE_minut},
  /*9*/   {SAMPLE_9,        SAMPLE_void,    SAMPLE_minut},
  /*10*/  {SAMPLE_10,       SAMPLE_void,    SAMPLE_minut},
  /*11*/  {SAMPLE_11,       SAMPLE_void,    SAMPLE_minut},
  /*12*/  {SAMPLE_12,       SAMPLE_void,    SAMPLE_minut},
  /*13*/  {SAMPLE_13,       SAMPLE_void,    SAMPLE_minut},
  /*14*/  {SAMPLE_14,       SAMPLE_void,    SAMPLE_minut},
  /*15*/  {SAMPLE_15,       SAMPLE_void,    SAMPLE_minut},
  /*16*/  {SAMPLE_16,       SAMPLE_void,    SAMPLE_minut},
  /*17*/  {SAMPLE_17,       SAMPLE_void,    SAMPLE_minut},
  /*18*/  {SAMPLE_18,       SAMPLE_void,    SAMPLE_minut},
  /*19*/  {SAMPLE_19,       SAMPLE_void,    SAMPLE_minut},
  
  /*20*/  {SAMPLE_20,       SAMPLE_void,    SAMPLE_minut},
  /*21*/  {SAMPLE_20,       SAMPLE_1w,      SAMPLE_minuta},
  /*22*/  {SAMPLE_20,       SAMPLE_2w,      SAMPLE_minuty},
  /*23*/  {SAMPLE_20,       SAMPLE_3,       SAMPLE_minuty},
  /*24*/  {SAMPLE_20,       SAMPLE_4,       SAMPLE_minuty},
  /*25*/  {SAMPLE_20,       SAMPLE_5,       SAMPLE_minut},
  /*26*/  {SAMPLE_20,       SAMPLE_6,       SAMPLE_minut},
  /*27*/  {SAMPLE_20,       SAMPLE_7,       SAMPLE_minut},
  /*28*/  {SAMPLE_20,       SAMPLE_8,       SAMPLE_minut},
  /*29*/  {SAMPLE_20,       SAMPLE_9,       SAMPLE_minut},
  
  /*30*/  {SAMPLE_30,       SAMPLE_void,    SAMPLE_minut},
  /*31*/  {SAMPLE_30,       SAMPLE_1w,      SAMPLE_minuta},
  /*32*/  {SAMPLE_30,       SAMPLE_2w,      SAMPLE_minuty},
  /*33*/  {SAMPLE_30,       SAMPLE_3,       SAMPLE_minuty},
  /*34*/  {SAMPLE_30,       SAMPLE_4,       SAMPLE_minuty},
  /*35*/  {SAMPLE_30,       SAMPLE_5,       SAMPLE_minut},
  /*36*/  {SAMPLE_30,       SAMPLE_6,       SAMPLE_minut},
  /*37*/  {SAMPLE_30,       SAMPLE_7,       SAMPLE_minut},
  /*38*/  {SAMPLE_30,       SAMPLE_8,       SAMPLE_minut},
  /*39*/  {SAMPLE_30,       SAMPLE_9,       SAMPLE_minut},
  
  /*40*/  {SAMPLE_40,       SAMPLE_void,    SAMPLE_minut},
  /*41*/  {SAMPLE_40,       SAMPLE_1w,      SAMPLE_minuta},
  /*42*/  {SAMPLE_40,       SAMPLE_2w,      SAMPLE_minuty},
  /*43*/  {SAMPLE_40,       SAMPLE_3,       SAMPLE_minuty},
  /*44*/  {SAMPLE_40,       SAMPLE_4,       SAMPLE_minuty},
  /*45*/  {SAMPLE_40,       SAMPLE_5,       SAMPLE_minut},
  /*46*/  {SAMPLE_40,       SAMPLE_6,       SAMPLE_minut},
  /*47*/  {SAMPLE_40,       SAMPLE_7,       SAMPLE_minut},
  /*48*/  {SAMPLE_40,       SAMPLE_8,       SAMPLE_minut},
  /*49*/  {SAMPLE_40,       SAMPLE_9,       SAMPLE_minut},
  
  /*50*/  {SAMPLE_50,       SAMPLE_void,    SAMPLE_minut},
  /*51*/  {SAMPLE_50,       SAMPLE_1w,      SAMPLE_minuta},
  /*52*/  {SAMPLE_50,       SAMPLE_2w,      SAMPLE_minuty},
  /*53*/  {SAMPLE_50,       SAMPLE_3,       SAMPLE_minuty},
  /*54*/  {SAMPLE_50,       SAMPLE_4,       SAMPLE_minuty}, 
  /*55*/  {SAMPLE_50,       SAMPLE_5,       SAMPLE_minut},
  /*56*/  {SAMPLE_50,       SAMPLE_6,       SAMPLE_minut},
  /*57*/  {SAMPLE_50,       SAMPLE_7,       SAMPLE_minut},
  /*58*/  {SAMPLE_50,       SAMPLE_8,       SAMPLE_minut},
  /*59*/  {SAMPLE_50,       SAMPLE_9,       SAMPLE_minut} };
  





static uint8_t temper_table[60][3] =
  /*0*/ { {SAMPLE_0,        SAMPLE_gradusov,    SAMPLE_void},
  /*1*/   {SAMPLE_1m,       SAMPLE_gradus,      SAMPLE_void},
  /*2*/   {SAMPLE_2m,       SAMPLE_gradusa,     SAMPLE_void},
  /*3*/   {SAMPLE_3,        SAMPLE_gradusa,     SAMPLE_void},
  /*4*/   {SAMPLE_4,        SAMPLE_gradusa,     SAMPLE_void},
  /*5*/   {SAMPLE_5,        SAMPLE_gradusov,    SAMPLE_void},
  /*6*/   {SAMPLE_6,        SAMPLE_gradusov,    SAMPLE_void},
  /*7*/   {SAMPLE_7,        SAMPLE_gradusov,    SAMPLE_void},
  /*8*/   {SAMPLE_8,        SAMPLE_gradusov,    SAMPLE_void},
  /*9*/   {SAMPLE_9,        SAMPLE_gradusov,    SAMPLE_void},
  /*10*/  {SAMPLE_10,       SAMPLE_gradusov,    SAMPLE_void},
  /*11*/  {SAMPLE_11,       SAMPLE_gradusov,    SAMPLE_void},
  /*12*/  {SAMPLE_12,       SAMPLE_gradusov,    SAMPLE_void},
  /*13*/  {SAMPLE_13,       SAMPLE_gradusov,    SAMPLE_void},
  /*14*/  {SAMPLE_14,       SAMPLE_gradusov,    SAMPLE_void},
  /*15*/  {SAMPLE_15,       SAMPLE_gradusov,    SAMPLE_void},
  /*16*/  {SAMPLE_16,       SAMPLE_gradusov,    SAMPLE_void},
  /*17*/  {SAMPLE_17,       SAMPLE_gradusov,    SAMPLE_void},
  /*18*/  {SAMPLE_18,       SAMPLE_gradusov,    SAMPLE_void},
  /*19*/  {SAMPLE_19,       SAMPLE_gradusov,    SAMPLE_void},
  
  /*20*/  {SAMPLE_20,       SAMPLE_gradusov,    SAMPLE_void},
  /*21*/  {SAMPLE_20,       SAMPLE_1m,          SAMPLE_gradus},
  /*22*/  {SAMPLE_20,       SAMPLE_2m,          SAMPLE_gradusa},
  /*23*/  {SAMPLE_20,       SAMPLE_3,           SAMPLE_gradusa},
  /*24*/  {SAMPLE_20,       SAMPLE_4,           SAMPLE_gradusa},
  /*25*/  {SAMPLE_20,       SAMPLE_5,           SAMPLE_gradusov},
  /*26*/  {SAMPLE_20,       SAMPLE_6,           SAMPLE_gradusov},
  /*27*/  {SAMPLE_20,       SAMPLE_7,           SAMPLE_gradusov},
  /*28*/  {SAMPLE_20,       SAMPLE_8,           SAMPLE_gradusov},
  /*29*/  {SAMPLE_20,       SAMPLE_9,           SAMPLE_gradusov},
  
  /*30*/  {SAMPLE_30,       SAMPLE_gradusov,    SAMPLE_void},
  /*31*/  {SAMPLE_30,       SAMPLE_1m,          SAMPLE_gradus},
  /*32*/  {SAMPLE_30,       SAMPLE_2m,          SAMPLE_gradusa},
  /*33*/  {SAMPLE_30,       SAMPLE_3,           SAMPLE_gradusa},
  /*34*/  {SAMPLE_30,       SAMPLE_4,           SAMPLE_gradusa},
  /*35*/  {SAMPLE_30,       SAMPLE_5,           SAMPLE_gradusov},
  /*36*/  {SAMPLE_30,       SAMPLE_6,           SAMPLE_gradusov},
  /*37*/  {SAMPLE_30,       SAMPLE_7,           SAMPLE_gradusov},
  /*38*/  {SAMPLE_30,       SAMPLE_8,           SAMPLE_gradusov},
  /*39*/  {SAMPLE_30,       SAMPLE_9,           SAMPLE_gradusov},
  
  /*40*/  {SAMPLE_40,       SAMPLE_gradusov,    SAMPLE_void},
  /*41*/  {SAMPLE_40,       SAMPLE_1m,          SAMPLE_gradus},
  /*42*/  {SAMPLE_40,       SAMPLE_2m,          SAMPLE_gradusa},
  /*43*/  {SAMPLE_40,       SAMPLE_3,           SAMPLE_gradusa},
  /*44*/  {SAMPLE_40,       SAMPLE_4,           SAMPLE_gradusa},
  /*45*/  {SAMPLE_40,       SAMPLE_5,           SAMPLE_gradusov},
  /*46*/  {SAMPLE_40,       SAMPLE_6,           SAMPLE_gradusov},
  /*47*/  {SAMPLE_40,       SAMPLE_7,           SAMPLE_gradusov},
  /*48*/  {SAMPLE_40,       SAMPLE_8,           SAMPLE_gradusov},
  /*49*/  {SAMPLE_40,       SAMPLE_9,           SAMPLE_gradusov},
  
  /*50*/  {SAMPLE_50,       SAMPLE_gradusov,    SAMPLE_void},
  /*51*/  {SAMPLE_50,       SAMPLE_1m,          SAMPLE_gradus},
  /*52*/  {SAMPLE_50,       SAMPLE_2m,          SAMPLE_gradusa},
  /*53*/  {SAMPLE_50,       SAMPLE_3,           SAMPLE_gradusa},
  /*54*/  {SAMPLE_50,       SAMPLE_4,           SAMPLE_gradusa}, 
  /*55*/  {SAMPLE_50,       SAMPLE_5,           SAMPLE_gradusov},
  /*56*/  {SAMPLE_50,       SAMPLE_6,           SAMPLE_gradusov},
  /*57*/  {SAMPLE_50,       SAMPLE_7,           SAMPLE_gradusov},
  /*58*/  {SAMPLE_50,       SAMPLE_8,           SAMPLE_gradusov},
  /*59*/  {SAMPLE_50,       SAMPLE_9,           SAMPLE_gradusov} };
