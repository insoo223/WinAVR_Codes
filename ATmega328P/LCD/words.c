/**************************************************************
 Target MCU & clock speed: ATmega328P @ 1Mhz internal
 Name    : words.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Created : May 17, 2015
 Updated : May 24, 2015

 Description: Get system compile time & date and display on LCD 2*16
    Button toggling to turn on or off the backlight of LCD

 HEX size[Byte]:

 Ref:
    Donald Weiman    (weimandn@alfredstate.edu)
    http://web.alfredstate.edu/weimandn/programming/lcd/ATmega328/LCD_code_gcc_4d.html
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "defines.h"

uint8_t wd=0;

// New words
uint8_t glbWords0[10][2][17];
/*
uint8_t glbWords0[10][2][17] =
    {
      {//0 "1234512345123456" "1234512345123456"
         //"petrified",       "cause2bcmStnlike"
           "f(DNA)>f(RNA)",   "chemical stblty"
      },
      {//1 "1234512345123456" "1234512345123456"
         //"arid",            "lckgSuffWtr/Rain"
           "gene region",     "cdngRegIntergen"
      },
      {//2 "1234512345123456" "1234512345123456"
         //"enliven",         "hgtn/intensify"
           "chromosome",      "gene repository"
      },
      {//3"1234512345123456" "1234512345123456"
         //"lurch",           "mv abruptly"
           "genome struc",    "unknown field"
      },
      {//4 "1234512345123456" "1234512345123456"
         //"oblivious to",     "lckgCnscsAwrnOf"
           "gene# genomeSize", "nonPrpt2LfCmlx"
      },
      {//5 "1234512345123456"   "1234512345123456"
         //"wtn a whisker of",  "extremelyClose 2"
           "homoSapiens",       "39Kgene/3.2Bnctp"
      },
      {//6 "1234512345123456"   "1234512345123456"
         //"colloid",           "mixWpropBtSolSus"
           "1gene-nProtein",    "diffGeneDensity"
      },
      {//7 "1234512345123456"   "1234512345123456"
         //"reticent",          "disclined2talk"
           "transcription",     "DNA->RNA@ncls"
      },
      {//8 "1234512345123456"   "1234512345123456"
         //"pry~apart",         "force2getSthOpen"
           "translation",       "ncacd2proteinNFO"
      },
      {//9 "1234512345123456"   "1234512345123456"
         //"esoteric",          "cnfnd2onyEnlgtnd"
           "ncltd triplet",     "1codon->1amnAcd"
         //"ribosome",          "triplet(codon) + tRNA"
      }
    }; //glbWords0[10][2][17]
*/

uint8_t glbWords1[10][2][17] =
    {
      {//0 "1234512345123456" "1234512345123456"
         //"snag",              "hiddenObstacle"
         //"mom",               "-9914-"
         "CHnambu",    "-5545 7876"
      },
      {//1 "1234512345123456" "1234512345123456"
         //"mission-agnostic",  "genApplicable"
         //"TJ",  "-4024-2946"
           "TJemo107-101",      "02-874-5046"
      },
      {//2 "1234512345123456" "1234512345123456"
           //"deep-stedCulture","fracturedRltnshp"
            "SunAh203-1503",    "032-518-2176"
      },
      {//3"1234512345123456" "1234512345123456"
         //"tumult",           "loudCnfsdNseByMs"
         //"SunAhMobile",    "017-361-0327"
         "BGOh",             "017-761-0322"

      },
      {//4 "1234512345123456" "1234512345123456"
        //"sobering",           "tending2mkSober"
        //"PHS",           "-5090-9139"
        "JungAh",    "-2743-3904"

      },
      {//5 "1234512345123456"   "1234512345123456"
         //"miscarrage",          "brthBbyAlrdDd@Wm"
         //"SJ_FlyzKssk@han",     "-4711-9816"
         "herold_kim@yahoo",    ".co.kr-3265-3846"
      },
      {//6 "1234512345123456"   "1234512345123456"
           //"class-actnLwSt",    "lawSuitTogether"
           //"JMH",               "-4312-2112"
           "DM/sdm1220@naver",  "-4499 7983"
      },
      {//7 "1234512345123456"   "1234512345123456"
           //"blister",           "fluidPocket"
           //"JK_jk1024@naver",   "-9249-4113"
           "SMnambu",              "-9409 1696"
      },
      {//8 "1234512345123456"   "1234512345123456"
           //"titter",            "laughQuietNrvsly"
           //"YS_yong2b@naver",      "055-587-4292"
           "RH/korean9191@",     "naver -3057 2733"
      },
      {//9 "1234512345123456"   "1234512345123456"
           //"line cook",         "a cook inKitchen"
           //"YS@Home",           "055-232-8027"
           "YS",                "-3581-8021"
      }
    }; //glbWords1[10][2][17]

// bio words set #00
/*
uint8_t bioWords00[10][2][17] =
    {
      {//0 "1234512345123456" "1234512345123456"
           "f(DNA)>f(RNA)",   "chemical stblty"
      },
      {//1 "1234512345123456" "1234512345123456"
           "gene region",     "cdngRegIntergen"
      },
      {//2 "1234512345123456" "1234512345123456"
           "chromosome",      "gene repository"
      },
      {//3"1234512345123456" "1234512345123456"
           "genome struc",    "unknown field"
      },
      {//4 "1234512345123456" "1234512345123456"
           "gene# genomeSize", "nonPrpt2LfCmlx"
      },
      {//5 "1234512345123456"   "1234512345123456"
           "homoSapiens",       "39Kgene/3.2Bnctp"
      },
      {//6 "1234512345123456"   "1234512345123456"
           "1gene-nProtein",    "diffGeneDensity"
      },
      {//7 "1234512345123456"   "1234512345123456"
           "transcription",     "DNA->RNA@ncls"
      },
      {//8 "1234512345123456"   "1234512345123456"
           "translation",       "ncacd2proteinNFO"
      },
      {//9 "1234512345123456"   "1234512345123456"
           "ncltd triplet",     "1codon->1amnAcd"
         //"ribosome",          "triplet(codon) + tRNA"
      }
    }; //bioWords00[10][2][17]
*/
// bio words set #01
uint8_t bioWords01[10][2][17] =
    {
      {//0 "1234512345123456" "1234512345123456"
           "cellChemReactn",  "bioMoleInteractn"
      },
      {//1 "1234512345123456" "1234512345123456"
           "bigSpace",        "lowReactnSpeed"
      },
      {//2 "1234512345123456" "1234512345123456"
           "whyOrganelle?",   "1)hiReactnSpeed"
      },
      {//3"1234512345123456" "1234512345123456"
           "whyOrganelle?",   "2)geneExpCtrl"
      },
      {//4 "1234512345123456" "1234512345123456"
           "trsc@n/trsl@ctp", "ctrlMvBetIntfc"
      },
      {//5 "1234512345123456"   "1234512345123456"
           "3)QC:trsc+proc",    "then, trsl"
      },
      //Ch1.5 Genome expression
      {//6 "1234512345123456"   "1234512345123456"
           "genoT->phenoT",     "geneNWintactn"
      },
      {//7 "1234512345123456"   "1234512345123456"
           "fwdGenetics",       "chgPhenoT->GenoT"
      },
      {//8 "1234512345123456"   "1234512345123456"
           "revGenetics",       "chgGenoT->PhenoT"
      },
      {//9 "1234512345123456"   "1234512345123456"
           "haploid/di-/tetr",  "yeast/hmspns/frg"
         //"ribosome",          "triplet(codon) + tRNA"
      }
    }; //bioWords01[10][2][17]

// bio words set #02
uint8_t bioWords02[10][2][17] =
    {
      {//0 "1234512345123456" "1234512345123456"
           "allele(Daeripza)","wild vs. mutant"
      },
      {//1 "1234512345123456" "1234512345123456"
           "mutation(dipld)", "recessive/domnt"
      },
      {//2 "1234512345123456" "1234512345123456"
           "mutation",        "pt/rarrg/del/ins"
      },
      {//3"1234512345123456" "1234512345123456"
           "null/missense",  "nonsense/silent"
      },
      {//4 "1234512345123456" "1234512345123456"
           "mono/polygenic",  "penetrance"
      },
      //Ch1.6 Genome evolution & phylogenetic tree
      {//5 "1234512345123456"   "1234512345123456"
           "phylogenetic Tr",   "rRNANctdSeqSmlty"
      },
      {//6 "1234512345123456"   "1234512345123456"
           "bctr/Eukrt",        "archaea"
      },
      //Ch2. Biomolecule
      {//7 "1234512345123456"   "1234512345123456"
           "4majBioMol:Nctd,",  "AmnAc/fatAc/Sgr"
      },
      //Ch2.3 non-covalent interaction
      {//8 "1234512345123456"   "1234512345123456"
           "atom:covalentBnd",  "mol:Non-covInt"
      },
      {//9 "1234512345123456"   "1234512345123456"
           "ionic/hydrogen",    "vanDerW,hydropho"
      },
    }; //bioWords02[10][2][17]

//--------------------------------
/*
void assignGlbWords0Bio0()
{
    uint8_t i,j;

    for (i=0; i<10; i++)
        for (j=0; j<2; j++)
            strcpy((char *)glbWords0[i][j], (char *)bioWords00[i][j]);
}//assignGlbWords0Bio0
*/
//--------------------------------
void assignGlbWords0Bio1()
{
    uint8_t i,j;

    for (i=0; i<10; i++)
        for (j=0; j<2; j++)
            strcpy((char *)glbWords0[i][j], (char *)bioWords01[i][j]);
}//assignGlbWords0Bio1
//--------------------------------
void assignGlbWords0Bio2()
{
    uint8_t i,j;

    for (i=0; i<10; i++)
        for (j=0; j<2; j++)
            strcpy((char *)glbWords0[i][j], (char *)bioWords02[i][j]);
}//assignGlbWords0Bio2
