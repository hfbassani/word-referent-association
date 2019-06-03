/* 
 * File:   main.cpp
 * Author: hans
 *
 * Created on 11 de Outubro de 2010, 07:25
 */

#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include <time.h>
#include <limits>
#include <unistd.h>
#include <stdexcept>
#include "MatUtils.h"
#include "MatMatrix.h"
#include "MatVector.h"
#include "Defines.h"
#include "DebugOut.h"
#include "ArffData.h"
#include "CrossSituationalTest.h"
#include "GDSSOMMW.h"
#include "StringHelper.h"
#include "LatinHypercubeSampling.h"
#include "ART2Context.h"

#define RG_MIN 0
#define RG_MAX 1

#define PAR_AT  0
#define PAR_LP  1
#define PAR_EB  2
#define PAR_EN  3
#define PAR_DSB 4
#define PAR_EDS 5
#define PAR_MDW 6
#define PAR_AW  7
#define PAR_DN  8

using namespace std;

int main(int argc, char** argv) {

    dbgThreshold(0);
    unsigned int seed = time(0);    

    string crossSTestFile = "";

    GDSSOMMW visualCBSOM(1);
    string visualCB = "";
    string imagePath = "";

    GDSSOMMW audioCBSOM(1);
    string audioCB = "";

    GDSSOMMW objectMap(1);

    //a_t	lp              e_b             e_n             dsbeta          e_ds            mwd             aw
    //0.995698	0.109644	0.269524	0.0617369	0.396285	0.822344	0.814887	203.594
    //0.995501	0.154104	0.583057	0.186989	0.60084         1.45844         0.625239	213.051

    //With Context:
    //a_t	lp              e_b             e_n             dsbeta          e_ds            mwd             aw
    //0.999904	0.00332746	0.60378         0.285152	0.248403	3.64209         0.715102	202.765
    //0.999896	0.0390911	0.20938         0.0286639	0.904144	2.50725         0.634525	211.32 (best)
    
//    objectMap.a_t	=	0.999896	;
//    objectMap.lp	=	0.0390911	;
//    objectMap.e_b	=	0.20938	;
//    objectMap.e_n	=	0.0286639	*objectMap.e_b;
//    objectMap.dsbeta	=	0.904144	;
//    objectMap.epsilon_ds	=	2.50725	;
//    objectMap.minwd	=	0.634525	;
//    objectMap.age_wins	=	10000;

    //a_t	lp              e_b	e_n             dsbeta          e_ds	mwd             aw
    //0.999924	0.0181181	0.14706	0.0108608	0.264555	4.42201	0.518132	206.006

    objectMap.a_t	=	0.999608	;
    objectMap.lp	=	0.175211	;
    objectMap.e_b	=	0.465091	;
    objectMap.e_n	=	0.0134102	*objectMap.e_b;
    objectMap.dsbeta	=	0.870879	;
    objectMap.epsilon_ds	=	1.31357	;
    objectMap.minwd	=	0.986745	;
    objectMap.age_wins	=	10000;

    ART2Context art2Context;    
    art2Context.parameters.rho = 0.895;//0.9572;
    art2Context.parameters.contextWeight = 0.3;
    art2Context.parameters.back = 0.99;
    art2Context.parameters.alpha = 0.001;
    art2Context.parameters.alpha_context = 0.5;
    art2Context.parameters.denorm_weight = 0.075;

    CFGFile cfgFile("/home/hans/Dropbox/Doutorado/Projetos/Data/houseobjects/CSWL/context/CST.ini");
    cfgFile >> art2Context.parameters;

    dbgToLogFile("/home/hans/Dropbox/Doutorado/Projetos/Data/houseobjects/CSWL/context/TestContext.csv", DebugOut::erase);

    int k = 5;
    int L = 10;

    int c;
    while ((c = getopt(argc, argv, "k:L:c:u:a:d:f:n:t:e:g:m:s:p:w:i:l:v:r:")) != -1)
        switch (c) {
            case 'k':
                k = atoi(optarg);
                break;
            case 'L':
                L = atoi(optarg);
                break;
            case 'c':
                    crossSTestFile.assign(optarg);
                break;
            case 'u':
                    visualCB.assign(optarg);
                break;
            case 'a':
                    audioCB.assign(optarg);
                break;                
            case 'd':
                    imagePath.assign(optarg);
                break;      
            case 'n':
            case 't':
                objectMap = atoi(optarg);
                break;
            case 'e':                
                objectMap.e_b = atof(optarg);
                break;
            case 'g':
                objectMap.e_n = atof(optarg);
                break;
            case 'm':
                objectMap.maxNodeNumber = atoi(optarg);
                break;
            case 's':
                objectMap.dsbeta = atof(optarg);
                break;
            case 'p':
                objectMap.epsilon_ds = atof(optarg);
                break;   
            case 'w':
                objectMap.minwd = atof(optarg);
                break;  
            case 'i':
                objectMap.age_wins = atof(optarg);
                break; 
            case 'l':
                objectMap.lp = atof(optarg);
                break;  
            case 'v':
                objectMap.a_t = atof(optarg);
                break;    
            case 'r':
                seed = atoi(optarg);
                break;
            case '?':
                if (optopt == 'f')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,"Unknown option character `\\x%x'.\n",optopt);
                return 1;
        }

    srand(seed);

    if (crossSTestFile == "") {
        dbgOut(0) << "option \"-c filename\" is required" << endl;
        return -1;
    }

    if (visualCB == "") {
        dbgOut(0) << "option \"-u filename\" is required" << endl;
        return -1;
    }

    if (imagePath == "") {
        dbgOut(0) << "option \"-d filename\" is required" << endl;
        return -1;
    }

    if (!visualCBSOM.readSOM(visualCB)) {
        dbgOut(0) << "Error reading: " << visualCB << endl;
        return -1;
    }

    if (audioCB == "") {
        dbgOut(0) << "option \"-a filename\" is required" << endl;
        return -1;
    }

    if (!audioCBSOM.readSOM(audioCB)) {
        dbgOut(0) << "Error reading: " << audioCB << endl;
        return -1;
    }

    CrossSituationalTest cst(&audioCBSOM, &visualCBSOM, &objectMap, &art2Context, imagePath);
    cst.setNormalizeInputs();
    std::string resultsFile = removeExtension(crossSTestFile) + ".csv";
    cst.setLogFile(resultsFile);    

    if (L>0) {
        
        MatMatrix<double> ranges(9, 2);
        ranges[PAR_AT][RG_MIN] = 0.999; ranges[PAR_AT][RG_MAX] = 0.99999; //at
        ranges[PAR_LP][RG_MIN] = 0.001; ranges[PAR_LP][RG_MAX] = 0.75; //LP
        ranges[PAR_EB][RG_MIN] = 0.001; ranges[PAR_EB][RG_MAX] = 0.1; //e_b
        ranges[PAR_EN][RG_MIN] = 0.001; ranges[PAR_EN][RG_MAX] = 0.5; //e_n
        ranges[PAR_DSB][RG_MIN] = 0.001; ranges[PAR_DSB][RG_MAX] = 1; //dsbeta
        ranges[PAR_EDS][RG_MIN] = 0.001; ranges[PAR_EDS][RG_MAX] = 0.1; //epsilon_ds
        ranges[PAR_MDW][RG_MIN] = 0; ranges[PAR_MDW][RG_MAX] = 1; //mindw
        ranges[PAR_AW][RG_MIN] = 250; ranges[PAR_AW][RG_MAX] = 250; //age_wins
        
        //ranges[PAR_DN][RG_MIN] = 0.01; ranges[PAR_DN][RG_MAX] = 0.5; //age_wins

        MatMatrix<double> lhs;
        LHS::getLHS(ranges, lhs, L);

        for (int j=0; j<lhs.rows(); j++) {

            dbgOut(0) << "running experiment " << (j+1) << " of " << lhs.rows() << endl;

            objectMap.a_t = lhs[j][PAR_AT];
            objectMap.lp  = lhs[j][PAR_LP];
            objectMap.e_b = lhs[j][PAR_EB];
            objectMap.e_n = lhs[j][PAR_EN]*objectMap.e_b;
            objectMap.dsbeta = lhs[j][PAR_DSB];
            objectMap.epsilon_ds = lhs[j][PAR_EDS];
            objectMap.minwd = lhs[j][PAR_MDW];
            objectMap.age_wins = lhs[j][PAR_AW];
            //art2Context.parameters.denorm_weight = ranges[PAR_DN][RG_MIN];

            for (int i=0; i<k; i++) {
                if (!cst.runCSTest(crossSTestFile)) {
                    dbgOut(0) << "fail to load " << crossSTestFile << endl;
                    return -1; 
                }
            }
            cst.logResults();
            cst.clearResults();
        }
    } else {
           for (int i=0; i<k; i++) {
               //dbgOut(0) << "running test " << (i+1) << " of " << k << endl;
                if (!cst.runCSTest(crossSTestFile)) {
                    dbgOut(0) << "fail to load " << crossSTestFile << endl;
                    return -1;
                }
                dbgOut(0) << endl;
                //cst.logResults(); 
                //cst.clearResults();
            }
            cst.closeLogFile();
    }
    
    sleep(2);
}
