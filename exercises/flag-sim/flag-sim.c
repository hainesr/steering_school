/****************************************************************************
 *  (C) Copyright 2007, 2008, University of Manchester, United Kingdom,
 *  all rights reserved.
 *
 *  This software was developed by the RealityGrid project
 *  (http://www.realitygrid.org), funded by the EPSRC under grants
 *  GR/R67699/01 and GR/R67699/02.
 *
 *  LICENCE TERMS
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS MATERIAL IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. THE ENTIRE RISK AS TO THE QUALITY
 *  AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE
 *  DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR
 *  CORRECTION.
 *
 *  Authors.......: Joanna Leng
 *                  Robert Haines
 ****************************************************************************/

/*
 * INTERNATIONAL AVS CENTRE - WARRANTY DISCLAIMER
 * Please read the file DISCLAIMER for conditions associated with this file.
 * avs@iavsc.org, www.iavsc.org
 */

/* ----------------------------------------------------------------------
 * FlagRaw Module
 * ----------------------------------------------------------------------
 * Description:
 *   Flag simulation module from AVS XP_DEMO CD.
 *   Converted from Dore' to AVS/Express by I. Curington
 *
 * Author: Ian Curington
 *
 * Modification History:
 *   12 April 1995 - ianc - CREATED express version
 *   16 April 1995 - ianc - fld call reorg
 *   23 May   1995 - ianc - fld call reorg 2
 *    8 June  1995 - ianc - optimization of field creation
 *   28 Sept  1995 - ianc - renamed sqrtl to sqrt_tbl
 *   13 Feb   1996 - ianc - changed to 3-arg omethod
 *   13 May   1996 - ianc - new parameter exposed
 *   13 Dec   1996 - ianc - fixed compile warning
 *   19 Mar   1998 - Paul Lever - modified for IAC repository
 *   30 Oct   2000 - Andrew Dodd - fixed compile warnings on Windows
 *   16 Nov   2007 - Joanna Leng - adapted for RealityGrid example
 *      Feb   2008 - Robert Haines - debugging/neatening/simplification
 *
 * ----------------------------------------------------------------------
 */

/***************************************************************************
 * Copyright (C) 1992, 1993
 * by Kubota Pacific Computer Inc.  All Rights Reserved.
 * This program is a trade secret of Kubota Pacific Computer Inc. and
 * it is not to be reproduced, published, disclosed to others, copied,
 * adapted, distributed, or displayed without the prior authorization
 * of Kubota Pacific Computer Inc.  Licensee agrees to attach or embed
 * this Notice on all copies of the program, including partial copies
 * or modified versions thereof.
 ****************************************************************************/

#include "flag.h"

#ifndef NO_STEERING
#include "ReG_Steer_Appside.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Search for string 'NOTE' for efficiancy notes 
 *
 * NOTE: Many divs can be omitted on horizontal and vertical (but not diagonal)
 * links if we assume a tight enough system that 'length d' ** will always be
 * close to 1.  
 */

int main(int argc, char** argv) {

  int n;
  int main_loop_count = 0;
  int data_vec_length;
#ifndef NO_STEERING
  /*
   * Variables declared in this section are needed for steering
   *
   * Add more things here to be used in subsequent
   * steering library calls.
   */

  int finished = REG_FALSE;
#else
  int main_loop_max = 10000;
  int output_freq = 100;
#endif

  FlagInfo flag_info;
  int flag_reset;

  /* initialise values in flag_info */
  flag_info.sizex = SIZEX;
  flag_info.sizey = SIZEY;
  flag_info.len1 = (SIZEX * SIZEY);
  flag_info.len3 = (3 * flag_info.len1);
  flag_info.xoff = 0;
  flag_info.yoff = flag_info.len1;
  flag_info.zoff = flag_info.len1 * 2;
  flag_info.traversal_counter = 0;
  flag_info.strength = 20.0;
  flag_info.wind_direction[0] = 0.5f;
  flag_info.wind_direction[1] = 1.0f;
  flag_info.windx = 0.0;
  flag_info.windy = 0.0;
  flag_info.windz = 0.0;
  flag_info.flag_release[RELEASE_TOP] = 0;
  flag_info.flag_release[RELEASE_BOTTOM] = 0;
  flag_info.flag_color = COLOR_SOLID;

  for(n = 0; n < flag_info.len3; n++) {
    flag_info.Vertices[n] = 0.0;
    flag_info.NodeData[n] = 0.0;
    flag_info.norm[n] = 0.0;
    flag_info.rel[n] = 0.0;
    flag_info.a[n] = 0.0;
  }
  
  for(n = 0; n < flag_info.len1; n++) {
    flag_info.b[n] = 0.0;
    flag_info.ld[n] = 0.0;
    flag_info.sf[n] = 0.0;
  }

  flag_reset = 0;

#ifndef NO_STEERING
  /*
   * Setup the steering library here.
   *
   * Initialise the library and parameters to steer/monitor.
   */

#else
  /* get run time and output frequency from command args */
  if(argc > 4) {
    fprintf(stderr, "Usage: %s [flag colour] [no. loops] [output freq]", argv[0]);
    fprintf(stderr, "\n\n  where flag colour can be:");
    fprintf(stderr, "\t0 - solid colour (default),\n");
    fprintf(stderr, "\t\t\t\t1 - coloured by velocity,\n");
    fprintf(stderr, "\t\t\t\t2 - coloured by force,\n");
    fprintf(stderr, "\t\t\t\t3 - coloured by force magnitude,\n");
    fprintf(stderr, "\t\t\t\t4 - texture mapped.\n");
    exit(EXIT_FAILURE);
  }

  if(argc > 1) {
    int colour = atoi(argv[1]);
    if(colour >= 0 && colour <= 4) {
      flag_info.flag_color = colour;
    }
  }

  if(argc > 2) {
    main_loop_max = atoi(argv[2]);
  }

  if(argc > 3) {
    output_freq = atoi(argv[3]);
  }
  else {
    output_freq = (int) main_loop_max / 100; 
  }
#endif

  /***********************
   * Run CFD Simulation  *
   ***********************/

  /* initialise the systems */
  init_sqrt(&flag_info);
  init_flag(&flag_info);
  calc_wind(&flag_info);
  createflag(&flag_info);

  /* get initial node data vector length */
  switch(flag_info.flag_color) {
  case COLOR_TEXTURE:
    data_vec_length = 2;
    break;
  case COLOR_FORCEMAG:
    data_vec_length = 1;
    break;
  case COLOR_VELOCITY:
  case COLOR_FORCE:
    data_vec_length = 3;
    break;
  case COLOR_SOLID:
    data_vec_length = 0;
    break;
  }

#ifndef NO_STEERING
  /* use a while-loop for indefinite run when steering */
  while(!finished) {

    if(main_loop_count % 10 == 0) { 
      printf("main loop count: %d\n", main_loop_count);
    }

    /* sleep for a bit as otherwise this runs too quickly! */
    usleep(10000); 

    if(flag_reset == 1) {
      /* reinitialise the systems */ 
      init_sqrt(&flag_info);
      init_flag(&flag_info);
      calc_wind(&flag_info);
      recreateflag(&flag_info);
      flag_reset = 0;
    }
#else
  /* use a for-loop for a bounded run when not steering */
  for(main_loop_count = 0; main_loop_count <= main_loop_max; main_loop_count++) {  
#endif

    forceflag(&flag_info);
    externalforces(&flag_info);
    moveflag(&flag_info);
    recreateflag(&flag_info);

#ifndef NO_STEERING
    /*
     * Talk to the steering library here
     * and handle the results.
     *
     * There is a lot to do here!
     * (hint: Steering_control)
     */

    /* need to increment the loop count manually */
    main_loop_count++;
#else
    /* output data files at output_freq */
    if(main_loop_count % output_freq == 0) {
      int i;
      int j;
      FILE* f_vertices;
      FILE* f_nodedata;
      char filename1[1000];
      char filename2[1000];
      int filename_pad = floor(log10(main_loop_max)) + 1;

      /* open vertices file and write */
      sprintf(filename1, "vertices-%0*2$d.dat", main_loop_count, filename_pad);
      if((f_vertices = fopen(filename1, "w")) == NULL) {
	printf("unable to open %s\n", filename1);
	exit(EXIT_FAILURE);
      }
      fprintf(f_vertices, "%d %d\n", SIZEX, SIZEY);
      j = 0;
      for(n = 0; n < (SIZEX * SIZEY); n++) {
	for(i = 0; i < 3; i++) {
	  fprintf(f_vertices, "%f ", flag_info.Vertices[j]);
	  j++;
	}
	fprintf(f_vertices, "\n");
      }
      if(fclose(f_vertices) == 1) {
	printf("unable to close %s\n", filename1);
	exit(EXIT_FAILURE);
      }
      
      if(data_vec_length > 0) {
	/* open nodedata file and write */
	sprintf(filename2, "nodedata-%0*2$d.dat", main_loop_count, filename_pad);
	if((f_nodedata = fopen(filename2, "w")) == NULL) {
	  printf("unable to open %s\n", filename2);
	  exit(EXIT_FAILURE);
	}
	fprintf(f_nodedata, "%d %d %d\n", SIZEX, SIZEY, data_vec_length);
	j = 0;
	for(n = 0; n < (SIZEX * SIZEY); n++) {
	  for(i = 0; i < data_vec_length; i++) {
	    fprintf(f_nodedata, "%f ", flag_info.NodeData[j]);
	    j++;
	  }
	  fprintf(f_nodedata, "\n");
	}
	if(fclose(f_nodedata) == 1) {
	  printf("unable to close %s\n", filename2);
	  exit(EXIT_FAILURE);
	}
      }

    }
#endif /* NO_STEERING */
  }

#ifndef NO_STEERING
  /*
   * Finalize steering library here!
   */
#endif

  return 0;
}
