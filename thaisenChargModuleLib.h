/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-27     Lenovo       the first version
 */
#ifndef APPLICATIONS_THAISENCHARGMODULELIB_H_
#define APPLICATIONS_THAISENCHARGMODULELIB_H_

typedef struct thasienModuleSetStruct
{
	uint8_t moduleProNo;
	uint8_t moduleGroupNum;
	uint8_t moduleSingleGroupNum[4];
}thasienModuleSetStruct;


typedef uint16_t (*thaisenModuleGetStatusFun_p)(uint8_t);

void thaisen_chargModule_Init(thaisenModuleGetStatusFun_p getStaFun,struct thaisenBMS_Charger_struct *chargerA,struct thaisenBMS_Charger_struct *chargerB,struct thasienModuleSetStruct *moduleS );

uint16_t  thaisenModuleGetStatusFun1(uint8_t cmd);


#endif /* APPLICATIONS_THAISENCHARGMODULELIB_H_ */

