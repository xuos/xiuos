 /*
* Copyright (c) 2020 AIIT Ubiquitous Team
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file sm9_test.c
* @brief tests of SM9
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#include <sm9_test.h>

/**
 * @brief test of sign and signature verify, data comes from SM9
 * 
 */
void SignAndVerifyTest()
{
    uint8_t ID_Alice[] = "Alice";
	uint8_t message[] = "Chinese IBS standard";
    uint8_t hid = 0x01, msglen = strlen((char*)message);
    uint8_t *Id_Alice_hid, *msg_w;

    big8w ks, t1, t2, r, h, h1, h2;
	G1point dsA;
	G2point Ppub_s, P;
	q12 t, g, w, u;

    Signature sig;

    //clock_t start, end, end_start;

    Id_Alice_hid = (unsigned char*)(malloc(5 + 1));

	ks.word[0] = 0x1F2DC5F4;
    ks.word[1] = 0x348A1D5B;
    ks.word[2] = 0x340F319F;
    ks.word[3] = 0x80CE0B66;
    ks.word[4] = 0x87E02CF4;
    ks.word[5] = 0x45CB54C5;
    ks.word[6] = 0x8459D785;
    ks.word[7] = 0x000130E7;

	r.word[0] = 0xDC4B1CBE;
    r.word[1] = 0xED648835;
    r.word[2] = 0xC662337A;
    r.word[3] = 0x2ED15975;
    r.word[4] = 0xD0096502;
    r.word[5] = 0x813203DF;
    r.word[6] = 0x16B06704;
    r.word[7] = 0x033C86;

    SM9Init();

	KPrintf("------------------------------below is ks---------------------------------\n");
    Big8wPrint(&ks);

	Ppub_s = G2PointMult(ks, P2);
    KPrintf("------------------------------below is Ppub_s-----------------------------\n");
    Big8wPrint(&Ppub_s.x.high);
    Big8wPrint(&Ppub_s.x.low);
    Big8wPrint(&Ppub_s.y.high);
    Big8wPrint(&Ppub_s.y.low);

    JoinIDhid(ID_Alice, 5, hid, Id_Alice_hid);

    t1 = Big8wAddMod(H(Id_Alice_hid, 5 + 1, 0x01), ks, curve.N);
    KPrintf("------------------------------below is t1---------------------------------\n");
    Big8wPrint(&t1);

    t2 = Big8wMultMod(ks, Big8wReverse(t1, curve.N), curve.N);
    KPrintf("------------------------------below is t2---------------------------------\n");
    Big8wPrint(&t2);

    dsA = G1pointMult(t2, P1);
    KPrintf("------------------------------below is dsA--------------------------------\n");
    Big8wPrint(&dsA.x);
    Big8wPrint(&dsA.y);

    g = BiLinearPairing(P1, Ppub_s);
    KPrintf("------------------------below is bilineapairing---------------------------\n");
    Q12Print(&g);

    w = Q12PowerMod(g, r);
    KPrintf("------------------------------below is w----------------------------------\n");
    Q12Print(&w);

    msg_w = (uint8_t*)(malloc(msglen + BIG8W_BYTESIZE * 12));
    JoinMsgW(message, msglen, &w, msg_w);
    h = H(msg_w, msglen + BIG8W_BYTESIZE * 12, 0x02);
    KPrintf("------------------------------below is h----------------------------------\n");
    Big8wPrint(&h);

    big8w L = Big8wMinusMod(r, h, curve.N);
    KPrintf("------------------------------below is L----------------------------------\n");
    Big8wPrint(&L);

    G1point S = G1pointMult(L, dsA);
    KPrintf("------------------------------below is S----------------------------------\n");
    Big8wPrint(&S.x);
    Big8wPrint(&S.y);
    KPrintf("\n");

    // verify the  signature
    g = BiLinearPairing(P1, Ppub_s);
    KPrintf("------------------------below is bilineapairing---------------------------\n");
    Q12Print(&g);

    t = Q12PowerMod(g, h);
    KPrintf("-----------------------------below is t-----------------------------------\n");
    Q12Print(&t);

    h1 = H(Id_Alice_hid, 5 + 1, 0x01);
    KPrintf("-----------------------------below is h1----------------------------------\n");
    Big8wPrint(&h1);

    P = G2PointAdd(Ppub_s, G2PointMult(h1, P2));
    KPrintf("------------------------------below is P----------------------------------\n");
    G2pointPrint(&P);

    u = BiLinearPairing(S, P);
    KPrintf("------------------------------below is u----------------------------------\n");
    Q12Print(&u);

    w = Q12MultMod(u, t);
    KPrintf("------------------------------below is w----------------------------------\n");
    Q12Print(&w);

    h2 = H(msg_w, msglen + BIG8W_BYTESIZE * 12, 0x02);
    KPrintf("------------------------------below is h2---------------------------------\n");
    Big8wPrint(&h2);
    KPrintf("------------------------------below is h----------------------------------\n");
    Big8wPrint(&h);
    
    if (Big8wEqual(&h2, &h)) 
        KPrintf("\nh2 = h, test verify success!\n");

    sig = SM9Sign(message, msglen, dsA, Ppub_s);
    if (SM9VerifySignature(ID_Alice, 5, hid, message, msglen, sig, Ppub_s))
        KPrintf("SM9 Sign and VerifySignature API run success!\n");

    
    /*
    start = clock();
    Ppub_s = G2PointMult(ks, P2);

    t1 = Big8wAddMod(H(Id_Alice_hid, 5 + 1, 0x01), ks, curve.N);

    t2 = Big8wMultMod(ks, Big8wReverse(t1, curve.N), curve.N);

    dsA = G1pointMult(t2, P1);

    g = BiLinearPairing(P1, Ppub_s);
    w = Q12PowerMod(g, r);

    JoinMsgW(message, msglen, &w, msg_w);
    h = H(msg_w, msglen + BIG8W_BYTESIZE * k, 0x02);

    L = Big8wMinusMod(r, h, curve.N);

    S = G1pointMult(L, dsA);

    end = clock();
    end_start = end - start;
	KPrintf("\n");
    KPrintf("runtime of sign in:  %d ms\n", end_start);
    */

    free(Id_Alice_hid);
    free(msg_w);

}
/**
 * @brief test Key exchange, data comes from SM9
 * 
 */
void SM9KeyExchangeTest()
{
    uint8_t ID_Alice[] = "Alice", ID_Bob[] = "Bob";
	uint8_t hid = 0x02;
	uint8_t Id_Alice_hid[5+1], ID_Bob_hid[3+1], message[100];

    uint32_t klen = 0x80, i;

	big8w ke, t1, t2, t3, t4, h1, rA, rB;
	G1point Ppub_e, RA, RB, QA, QB;
	G2point deA, deB;
	q12 g1, g2, g3;

	uint8_t *strA, *strB, *SKA, *SKB, *SA, *SB, *S1, *S2;
	strA = (uint8_t*)(malloc(5 + 3 + BIG8W_BYTESIZE * 2 * 2 + BIG8W_BYTESIZE * 12 * 3));
	strB = (uint8_t*)(malloc(5 + 3 + BIG8W_BYTESIZE * 2 * 2 + BIG8W_BYTESIZE * 12 * 3));
	SKA = (uint8_t*)(malloc(klen));
	SKB = (uint8_t*)(malloc(klen));
	SA = (uint8_t*)(malloc(256/8));
	SB = (uint8_t*)(malloc(256/8));
	S1 = (uint8_t*)(malloc(256/8));
	S2 = (uint8_t*)(malloc(256/8));

    memset(message, 0x00, 100);

    ke.word[7] = 0x02E65B;
    ke.word[6] = 0x0762D042;
    ke.word[5] = 0xF51F0D23;
    ke.word[4] = 0x542B13ED;
    ke.word[3] = 0x8CFA2E9A;
    ke.word[2] = 0x0E720636;
    ke.word[1] = 0x1E013A28;
    ke.word[0] = 0x3905E31F;

	rA.word[7] = 0x5879;
	rA.word[6] = 0xDD1D51E1;
	rA.word[5] = 0x75946F23;
	rA.word[4] = 0xB1B41E93;
	rA.word[3] = 0xBA31C584;
	rA.word[2] = 0xAE59A426;
	rA.word[1] = 0xEC1046A4;
	rA.word[0] = 0xD03B06C8;

	rB.word[7] = 0x018B98;
	rB.word[6] = 0xC44BEF9F;
	rB.word[5] = 0x8537FB7D;
	rB.word[4] = 0x071B2C92;
	rB.word[3] = 0x8B3BC65B;
	rB.word[2] = 0xD3D69E1E;
	rB.word[1] = 0xEE213564;
	rB.word[0] = 0x905634FE;
	
    SM9Init();

	Ppub_e = G1pointMult(ke, P1);
    KPrintf("------------------------------below is Ppub_e-----------------------------\n");
    Big8wPrint(&Ppub_e.x);
    Big8wPrint(&Ppub_e.y);

    JoinIDhid(ID_Alice, 5, hid, Id_Alice_hid);
    t1 = Big8wAddMod(H(Id_Alice_hid, 5 + 1, 0x01), ke, curve.N);
    KPrintf("-----------------------------below is t1----------------------------------\n");
    Big8wPrint(&t1);

    t2 = Big8wMultMod(ke, Big8wReverse(t1, curve.N), curve.N);
    KPrintf("-----------------------------below is t2----------------------------------\n");
    Big8wPrint(&t2);

    deA = G2PointMult(t2, P2);
    KPrintf("------------------------------below is deA--------------------------------\n");
    Big8wPrint(&deA.x.high);
    Big8wPrint(&deA.x.low);
    Big8wPrint(&deA.y.high);
    Big8wPrint(&deA.y.low);

	JoinIDhid(ID_Bob, 3, hid, ID_Bob_hid);

	t3 = Big8wAddMod(H(ID_Bob_hid, 3 + 1, 0x01), ke, curve.N);
	KPrintf("-----------------------------below is t3----------------------------------\n");
    Big8wPrint(&t3);

	t4 = Big8wMultMod(ke, Big8wReverse(t3, curve.N), curve.N);
	KPrintf("-----------------------------below is t4----------------------------------\n");
    Big8wPrint(&t4);

	deB = G2PointMult(t4, P2);
	KPrintf("------------------------------below is deB--------------------------------\n");
    Big8wPrint(&deB.x.high);
    Big8wPrint(&deB.x.low);
    Big8wPrint(&deB.y.high);
    Big8wPrint(&deB.y.low);

	JoinIDhid(ID_Bob, 3, hid, ID_Bob_hid);

	h1 = H(ID_Bob_hid, 3 + 1, 0x01);
	KPrintf("-----------------------------below is h1----------------------------------\n");
    Big8wPrint(&h1);

	QB = G1pointAdd(Ppub_e, G1pointMult(h1, P1));
	KPrintf("-----------------------------below is QB----------------------------------\n");
    Big8wPrint(&QB.x);
	Big8wPrint(&QB.y);

	RA = G1pointMult(rA, QB);
	KPrintf("-----------------------------below is RA----------------------------------\n");
    Big8wPrint(&RA.x);
	Big8wPrint(&RA.y);

	JoinIDhid(ID_Alice, 5, hid, Id_Alice_hid);
	h1 = H(Id_Alice_hid, 5 + 1, 0x01);
	KPrintf("-----------------------------below is h1----------------------------------\n");
    Big8wPrint(&h1);

	QA = G1pointAdd(Ppub_e, G1pointMult(h1, P1));
	KPrintf("-----------------------------below is QA----------------------------------\n");
    Big8wPrint(&QA.x);
	Big8wPrint(&QA.y);

	RB = G1pointMult(rB, QA);
	KPrintf("-----------------------------below is RB----------------------------------\n");
    Big8wPrint(&RB.x);
	Big8wPrint(&RB.y);

	g1 = BiLinearPairing(RA, deB);
	KPrintf("-----------------------------below is g1----------------------------------\n");
	Q12Print(&g1);

	g2 = BiLinearPairing(Ppub_e, P2);
	g2 = Q12PowerMod(g2, rB);
	KPrintf("-----------------------------below is g2----------------------------------\n");
	Q12Print(&g2);

	g3 = Q12PowerMod(g1, rB);
	KPrintf("-----------------------------below is g3----------------------------------\n");
	Q12Print(&g3);

    JoinIDAIDBRARBg123(ID_Alice, 5, ID_Bob, 3, &RA, &RB, &g1, &g2, &g3, strA);

    KDF(strA, 5 + 3 + BIG8W_BYTESIZE * 2 * 2 + BIG8W_BYTESIZE * 12 * 3, klen, SKB);
    KPrintf("-----------------------------below is SKB---------------------------------\n");
	for (i = 0; i < klen/8; i++){
        KPrintf("%02x", SKB[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
	KPrintf("\n");

	KPrintf("-----------------------------below is SB----------------------------------\n");
	HashTwice(ID_Alice, 5, ID_Bob, 3, &RA, &RB, &g1, &g2, &g3, 0x82, SB);
	HashTwice(ID_Alice, 5, ID_Bob, 3, &RA, &RB, &g1, &g2, &g3, 0x83, S2);
	for (i = 0; i < 256/8; i++){
        KPrintf("%02x", SB[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
	KPrintf("\n");

	g1 = BiLinearPairing(Ppub_e, P2);
	g1 = Q12PowerMod(g1, rA);
	KPrintf("-----------------------------below is g1----------------------------------\n");
	Q12Print(&g1);

	g2 = BiLinearPairing(RB, deA);
	KPrintf("-----------------------------below is g2----------------------------------\n");
	Q12Print(&g2);

	g3 = Q12PowerMod(g2, rA);
	KPrintf("-----------------------------below is g3----------------------------------\n");
	Q12Print(&g3);

	KPrintf("-----------------------------below is S1----------------------------------\n");
	HashTwice(ID_Alice, 5, ID_Bob, 3, &RA, &RB, &g1, &g2, &g3, 0x82, S1);

	for (i = 0; i < 256/8; i++){
        KPrintf("%02x", S1[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
	KPrintf("\n");

	KDF(strA, 5 + 3 + BIG8W_BYTESIZE * 2 * 2 + BIG8W_BYTESIZE * 12 * 3, klen, SKA);
	KPrintf("-----------------------------below is SKA---------------------------------\n");
	for (i = 0; i < klen/8; i++){
        KPrintf("%02x", SKA[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
	printf("\n");

	KPrintf("-----------------------------below is SA----------------------------------\n");
	HashTwice(ID_Alice, 5, ID_Bob, 3, &RA, &RB, &g1, &g2, &g3, 0x83, SA);
	for (i = 0; i < 256/8; i++){
        KPrintf("%02x", SA[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
	KPrintf("\n");

	KPrintf("-----------------------------below is S2----------------------------------\n");
	for (i = 0; i < 256/8; i++){
        KPrintf("%02x", S2[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
    KPrintf("\n");

    // Following is test of API, random big number generated in SM9KeyExchangeProduceR, so result is different from the former.
    // To get the same result, you should delete the line "*r = RandomNumGenerate();" in function SM9KeyExchangeR. (or set as note)

    KPrintf("---------------------------SM9 key exchange API test----------------------\n");

    SM9KeyExchangeProduceR(ID_Bob, 3, &rA, &RA, Ppub_e);
    KPrintf("-----------------------------below is RA----------------------------------\n");
    Big8wPrint(&RA.x);
	Big8wPrint(&RA.y);

    SM9KeyExchangeProduceR(ID_Alice, 5, &rB, &RB, Ppub_e);
    KPrintf("-----------------------------below is RB----------------------------------\n");
    Big8wPrint(&RB.x);
	Big8wPrint(&RB.y);

    SM9KeyExchangeProduceKey(&RA, &RB, &rA, klen, ID_Alice, 5, ID_Bob, 3, &g1, &g2, &g3, SKA, true, Ppub_e, deA);
    KPrintf("-----------------------------below is g1----------------------------------\n");
	Q12Print(&g1);

	KPrintf("-----------------------------below is g2----------------------------------\n");
	Q12Print(&g2);

	KPrintf("-----------------------------below is g3----------------------------------\n");
	Q12Print(&g3);

    KPrintf("-----------------------------below is SKA---------------------------------\n");
	for (i = 0; i < klen/8; i++){
        KPrintf("%02x", SKA[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
	KPrintf("\n");

    // g1,g2,g3 changed
    // SM9KeyExchangeProduceKey(&RA, &RB, &rB, klen, ID_Alice, 5, ID_Bob, 3, &g1, &g2, &g3, SKB, false, Ppub_e, deB);

    SM9KeyExchangeVerifyKey(&g1, &g2, &g3, &RA, &RB, ID_Alice, 5, ID_Bob, 3, S1, SA);
    KPrintf("-----------------------------below is SA----------------------------------\n");
	for (i = 0; i < 256/8; i++){
        KPrintf("%02x", SA[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
	KPrintf("\n");

	KPrintf("-----------------------------below is S1----------------------------------\n");
	for (i = 0; i < 256/8; i++){
        KPrintf("%02x", S1[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
    KPrintf("\n");

    SM9KeyExchangeVerifyKey(&g1, &g2, &g3, &RA, &RB, ID_Alice, 5, ID_Bob, 3, SB, S2);
    KPrintf("-----------------------------below is SB----------------------------------\n");
	for (i = 0; i < 256/8; i++){
        KPrintf("%02x", SB[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
	KPrintf("\n");

	KPrintf("-----------------------------below is S2----------------------------------\n");
	for (i = 0; i < 256/8; i++){
        KPrintf("%02x", S2[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
    KPrintf("\n");
    
    free(strA);
    free(strB);
    free(SKA);
    free(SKB);
    free(SA);
    free(SB);
    free(S1);
    free(S2);
}
/**
 * @brief test of key package and depackage, data comes from SM9
 * 
 */
void SM9PackDepackTest()
{
    uint32_t i, c_w_id_len, klen = 0x100;
	big8w ke, t1, t2, r;
	G1point Ppub_e, QB, C;
	G2point deB;
	q12 g, w;
	
	uint8_t hid = 0x03;
    uint8_t ID_Bob[] = "Bob";
	uint8_t* ID_Bob_hid;
	uint8_t* K_encap, * c_w_id;

    c_w_id_len = BIG8W_BYTESIZE * 2 + BIG8W_BYTESIZE * 12 + 3;

	ID_Bob_hid= (unsigned char*)(malloc(sizeof(char) * (3 + 1)));
    K_encap = (unsigned char*)(malloc(32));
    c_w_id = (unsigned char*)(malloc(c_w_id_len));

	ke.word[7] = 0x01EDEE;
    ke.word[6] = 0x3778F441;
    ke.word[5] = 0xF8DEA3D9;
    ke.word[4] = 0xFA0ACC4E;
    ke.word[3] = 0x07EE36C9;
    ke.word[2] = 0x3F9A0861;
    ke.word[1] = 0x8AF4AD85;
    ke.word[0] = 0xCEDE1C22;

	r.word[7] = 0x7401;
    r.word[6] = 0x5F8489C0;
    r.word[5] = 0x1EF42704;
    r.word[4] = 0x56F9E647;
    r.word[3] = 0x5BFB602B;
    r.word[2] = 0xDE7F33FD;
    r.word[1] = 0x482AB4E3;
    r.word[0] = 0x684A6722;

    SM9Init();

	Ppub_e = G1pointMult(ke, P1);
    KPrintf("-----------------------------below is Ppub_e------------------------------\n");
    Big8wPrint(&Ppub_e.x);
    Big8wPrint(&Ppub_e.y);

    JoinIDhid(ID_Bob, 3, hid, ID_Bob_hid);
    t1 = H(ID_Bob_hid, 3 + 1, 0x01);
    KPrintf("-----------------------------below is H1()--------------------------------\n");
    Big8wPrint(&t1);
    t1 = Big8wAddMod(t1, ke, curve.N);
    KPrintf("-----------------------------below is t1----------------------------------\n");
    Big8wPrint(&t1);

    t1 = Big8wReverse(t1, curve.N);
    t2 = Big8wMultMod(ke, t1, curve.N);
    KPrintf("-----------------------------below is t2----------------------------------\n");
    Big8wPrint(&t2);

    deB = G2PointMult(t2, P2);
	KPrintf("------------------------------below is deB--------------------------------\n");
    Big8wPrint(&deB.x.high);
    Big8wPrint(&deB.x.low);
    Big8wPrint(&deB.y.high);
    Big8wPrint(&deB.y.low);

    QB = G1pointAdd(
        G1pointMult(H(ID_Bob_hid, 3 + 1, 0x01), P1),
        Ppub_e);
	KPrintf("-----------------------------below is QB----------------------------------\n");
    Big8wPrint(&QB.x);
	Big8wPrint(&QB.y);

    C =  G1pointMult(r, QB);
    KPrintf("-----------------------------below is C----------------------------------\n");
    Big8wPrint(&C.x);
    Big8wPrint(&C.y);

    g = BiLinearPairing(Ppub_e, P2);
    KPrintf("-----------------------------below is g----------------------------------\n");
    Q12Print(&g);

    w = Q12PowerMod(g, r);
    KPrintf("-----------------------------below is w----------------------------------\n");
    Q12Print(&w);

    JoinCwID(&C, &w, ID_Bob, 3, c_w_id);

    KDF(c_w_id, c_w_id_len, klen, K_encap);
    KPrintf("-----------------------------below is K-----------------------------------\n");
    for (i = 0; i < klen / 8; i++){
        KPrintf("%02x", K_encap[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
    KPrintf("\n");

	w = BiLinearPairing(C, deB);
	KPrintf("-----------------------------below is w'----------------------------------\n");
    Q12Print(&w);

	JoinCwID(&C, &w, ID_Bob, 3, c_w_id);

    KDF(c_w_id, c_w_id_len, klen, K_encap);
    KPrintf("-----------------------------below is K'----------------------------------\n");
    for (i = 0; i < klen / 8; i++){
        KPrintf("%02x", K_encap[i]);
        if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
    KPrintf("\n");
    KPrintf("pack and depack done\n");

    free(ID_Bob_hid);
    free(K_encap);
    free(c_w_id);
}
/**
 * @brief test of encrypt and decrypt with KDF and SM4, data comes from SM9
 * 
 */
void SM9EncryptDecryptTest()
{
	big8w r, ke, t1, t2;
	G1point Ppub_e, QB, C1;
	G2point deB;
    uint32_t i;
    uint32_t K1_len = 0x80, K2_len = 0x100, mlen = 0xA0;
    uint32_t klen = mlen + K2_len;
    uint32_t c_w_id_len = BIG8W_BYTESIZE * 2 + BIG8W_BYTESIZE * 12 + 3; // len(ID_Bob) = 3;
    q12 g, w;

	unsigned char hid = 0x03;
	unsigned char ID_Bob[] = "Bob";
	unsigned char ID_Bob_hid[4];
	unsigned char message[] = "Chinese IBE standard";
    unsigned char *C_w_id;
    unsigned char *K, *C2, *C3, *C2K2, *C, *C_SM4;
    unsigned char sm4msg[32];

    memset(sm4msg, 0x00, 32);

    unsigned int Cbyteslen_KDF = (BIG8W_BYTESIZE * 2) + (256 / 8) + (mlen / 8);
    unsigned int Cbyteslen_SM4 = (BIG8W_BYTESIZE * 2) + (256 / 8) + ((mlen / 128) + 1) * (128 / 8);

    C_w_id = (uint8_t*)(malloc(c_w_id_len));
	K = (uint8_t*)(malloc(((klen >> 8) + 1) * (256 / 8)));
	C2 = (uint8_t*)(malloc(mlen / 8));
	C3 = (uint8_t*)(malloc(256 / 8));
    C2K2 = (uint8_t*)(malloc(mlen/8 + K2_len/8));
	C = (uint8_t*)(malloc(Cbyteslen_KDF));
    C_SM4 = (uint8_t *)(malloc(Cbyteslen_SM4));

    r.word[7] = 0xAAC0;
	r.word[6] = 0x541779C8;
	r.word[5] = 0xFC45E3E2;
	r.word[4] = 0xCB25C12B;
	r.word[3] = 0x5D2576B2;
	r.word[2] = 0x129AE8BB;
	r.word[1] = 0x5EE2CBE5;
	r.word[0] = 0xEC9E785C;

	ke.word[7] = 0x01EDEE;
    ke.word[6] = 0x3778F441;
    ke.word[5] = 0xF8DEA3D9;
    ke.word[4] = 0xFA0ACC4E;
    ke.word[3] = 0x07EE36C9;
    ke.word[2] = 0x3F9A0861;
    ke.word[1] = 0x8AF4AD85;
    ke.word[0] = 0xCEDE1C22;

    SM9Init();

    Ppub_e = G1pointMult(ke, P1);
	KPrintf("-----------------------------below is Ppub_e------------------------------\n");
    Big8wPrint(&Ppub_e.x);
    Big8wPrint(&Ppub_e.y);

	JoinIDhid(ID_Bob, 3, hid, ID_Bob_hid);
	t1 = Big8wAddMod(H(ID_Bob_hid, 3 + 1, 0x01), ke, curve.N);
	KPrintf("-----------------------------below is t1----------------------------------\n");
    Big8wPrint(&t1);

	t1 = Big8wReverse(t1, curve.N);
    t2 = Big8wMultMod(ke, t1, curve.N);
    KPrintf("-----------------------------below is t2----------------------------------\n");
    Big8wPrint(&t2);

    deB = G2PointMult(t2, P2);
	KPrintf("------------------------------below is deB--------------------------------\n");
    Big8wPrint(&deB.x.high);
    Big8wPrint(&deB.x.low);
    Big8wPrint(&deB.y.high);
    Big8wPrint(&deB.y.low);

	QB = G1pointAdd(
        G1pointMult(H(ID_Bob_hid, 3 + 1, 0x01), P1),
        Ppub_e);
	KPrintf("-----------------------------below is QB----------------------------------\n");
    Big8wPrint(&QB.x);
	Big8wPrint(&QB.y);

	C1 = G1pointMult(r, QB);
	KPrintf("-----------------------------below is C1----------------------------------\n");
    Big8wPrint(&C1.x);
	Big8wPrint(&C1.y);

	g = BiLinearPairing(Ppub_e, P2);
	KPrintf("-----------------------------below is g-----------------------------------\n");
	Q12Print(&g);

	w = Q12PowerMod(g, r);
	KPrintf("-----------------------------below is w-----------------------------------\n");
	Q12Print(&w);

    JoinCwID(&C1, &w, ID_Bob, 3, C_w_id);
	KDF(C_w_id, c_w_id_len, klen, K);
	KPrintf("-----------------------------below is K-----------------------------------\n");
	for (i = 0; i < klen / 8; i++) {
		KPrintf("%02x", K[i]);
		if (((i+1)&0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
	}
	KPrintf("\n");

    XOR(message, mlen / 8, K, C2);

	KPrintf("-----------------------------below is C2----------------------------------\n");
	for (i = 0; i < mlen / 8; i++) {
		KPrintf("%02x", C2[i]);
		if (((i + 1) & 0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
	}
	KPrintf("\n");

	for (i = 0; i < mlen / 8; i++)
		C2K2[i] = C2[i];
	for (; i < (mlen / 8) + (K2_len / 8); i++)
		C2K2[i] = K[i];

	sm3(C2K2, (mlen / 8) + (K2_len / 8), C3);
	KPrintf("----------------------------below is C3-----------------------------------\n");
	for (i = 0; i < 256/8; i++) {
		KPrintf("%02x", C3[i]);
		if (((i + 1) & 0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
	}
	KPrintf("\n");

	i = 0;
	Big8wIntou8string(&C1.x, C, i);
	i += BIG8W_BYTESIZE;
	Big8wIntou8string(&C1.y, C, i);
	i += BIG8W_BYTESIZE;

	for (; i < BIG8W_BYTESIZE * 2 + (256 / 8); i++)
		C[i] = C3[i - BIG8W_BYTESIZE * 2];

	for (; i < BIG8W_BYTESIZE * 2 + (256 / 8) + (mlen / 8); i++)
		C[i] = C2[i - (BIG8W_BYTESIZE * 2 + 256 / 8)];

	KPrintf("----------------------------below is C------------------------------------\n");
	for (i = 0; i < BIG8W_BYTESIZE * 2 + (256 / 8) + (mlen / 8); i++) {
		KPrintf("%02x", C[i]);
		if (((i + 1) & 0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
	}
	KPrintf("\n");

    // decrypt
	w = BiLinearPairing(C1, deB);
	KPrintf("----------------------------below is w'-----------------------------------\n");
	Q12Print(&w);

	JoinCwID(&C1, &w, ID_Bob, 3, C_w_id);
	KDF(C_w_id, c_w_id_len, klen, K);
	KPrintf("----------------------------below is K'----------------------------------\n");
	for (i = 0; i < (klen / 8); i++) {
		KPrintf("%02x", K[i]);
		if (((i + 1) & 0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
	}
	KPrintf("\n");

	XOR(C2, mlen / 8, K, message);

	KPrintf("----------------------------below is M'----------------------------------\n");
	for (i = 0; i < mlen / 8; i++) {
		KPrintf("%02x", message[i]);
		if (((i + 1) & 0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
	}
	KPrintf("\n");

	for (i = 0; i < mlen / 8; i++)
		C2K2[i] = C2[i];
	for (; i < (mlen / 8) + (K2_len / 8); i++)
		C2K2[i] = K[i];

	sm3(C2K2, (mlen / 8) + (K2_len / 8), C3);
	KPrintf("----------------------------below is u-----------------------------------\n");
	for (i = 0; i < 256 / 8; i++) {
		KPrintf("%02x", C3[i]);
		if (((i + 1) & 0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
	}
	KPrintf("\n");

	KPrintf("decrypted message:\n%s\n", message);


    KPrintf("-------------SM9EncryptWithKDF and SM9DecryptWithKDF test----------------\n");
    SM9EncryptWithKDF(message, mlen, K2_len, ID_Bob, 3, hid, Ppub_e, C);
    KPrintf("------------------below is C, encrypted message--------------------------\n");
	for (i = 0; i < Cbyteslen_KDF; i++) {
		KPrintf("%02x", C[i]);
		if (((i + 1) & 0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
	}
	KPrintf("\n");

    SM9DecryptWithKDF(ID_Bob, 3, message, 20 * 8, K2_len, C, deB);
    KPrintf("---------------below is M, decrypted message in test.c-------------------\n");
    KPrintf("message:\n%s\n", message);

    KPrintf("-------------SM9EncryptWithSM4 and SM9DecryptWithSM4 test----------------\n");
    SM9EncryptWithSM4(message, mlen, K1_len, K2_len, ID_Bob, 3, hid, Ppub_e, C_SM4);
    KPrintf("---------------below is C_SM4 in test.c, encrypted message---------------\n");
	for (i = 0; i < Cbyteslen_SM4; i++) {
		KPrintf("%02x", C_SM4[i]);
		if (((i + 1) & 0x3) == 0)
			KPrintf(" ");
        if (((i + 1) % 32) == 0)
            KPrintf("\n");
    }
	KPrintf("\n");

    if (!SM9DecryptWithSM4(ID_Bob, 3, sm4msg, (mlen / 8), K1_len, K2_len, C_SM4, Cbyteslen_SM4, deB))
        KPrintf("SM9DecryptWithSM4 failed\n");
    KPrintf("---------------------below is M, decrypted message-----------------------\n");
    for (i = 0; i < (mlen / 8); i++)
        KPrintf("%c", sm4msg[i]);
    KPrintf("\n");

    free(C_w_id);
    free(K);
    free(C2);
    free(C2K2);
    free(C3);
    free(C);
    free(C_SM4);
}

// int main()
// {
//     //SignAndVerifyTest();
//     SM9KeyExchangeTest();
//     //SM9PackDepackTest();
//     //SM9EncryptDecryptTest();
// }