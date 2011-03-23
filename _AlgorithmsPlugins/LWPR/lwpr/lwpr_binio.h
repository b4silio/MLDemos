/*********************************************************************
LWPR: A library for incremental online learning
Copyright (C) 2007  Stefan Klanke, Sethu Vijayakumar
Contact: sethu.vijayakumar@ed.ac.uk

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either 
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/


/** \file lwpr_binio.h
   \brief Prototypes for binary-IO related LWPR subroutines
   
   This header file describes routines for reading and writing LWPR 
   models from and to binary files. Please note that these functions
   assume that the models are stored and loaded on the same machine 
   architecture (i.e. endianness, word length), and NO conversion 
   is done. If you need to exchange LWPR models across different 
   architectures, use the routines in lwpr_xml.h
   
   The file format is as follows: 
   
   <TABLE>
   <TR><TH>Element description</TH><TH>Size of element</TH></TR>
   <TR><TD>"LWPR"</TD><TD>4 bytes</TD></TR>
   <TR><TD>BINIO version      </TD><TD>1 integer (negative since LWPR 1.1)</TD></TR>   
   <TR><TD>nIn                </TD><TD>1 integer </TD></TR>
   <TR><TD>nIn                </TD><TD>1 integer </TD></TR>
   <TR><TD>nOut               </TD><TD>1 integer </TD></TR>
   <TR><TD>kernel             </TD><TD>1 integer </TD></TR>
   <TR><TD>length of 'name' string (may be 0)</TD><TD>integer </TD></TR>
   <TR><TD>name of the model  </TD><TD>nr. of bytes as determined by previous integer </TD></TR>
   <TR><TD>n_data             </TD><TD>1 integer </TD></TR>
   <TR><TD>mean_x             </TD><TD>nIn doubles </TD></TR>
   <TR><TD>var_x              </TD><TD>nIn doubles </TD></TR>
   <TR><TD>diag_only          </TD><TD>1 integer </TD></TR>
   <TR><TD>update_D           </TD><TD>1 integer (was not present in LWPR 1.0 due to a bug)</TD></TR>
   <TR><TD>meta               </TD><TD>1 integer </TD></TR>
   <TR><TD>meta_rate          </TD><TD>1 double </TD></TR>
   <TR><TD>penalty            </TD><TD>1 double </TD></TR>
   <TR><TD>init_alpha         </TD><TD>nIn*nIn doubles </TD></TR>
   <TR><TD>norm_in            </TD><TD>nIn doubles </TD></TR>
   <TR><TD>norm_out           </TD><TD>nOut doubles </TD></TR>
   <TR><TD>init_D             </TD><TD>nIn*nIn doubles </TD></TR>
   <TR><TD>init_M             </TD><TD>nIn*nIn doubles </TD></TR>
   <TR><TD>w_gen              </TD><TD>1 double </TD></TR>
   <TR><TD>w_prune            </TD><TD>1 double </TD></TR>
   <TR><TD>init_lambda        </TD><TD>1 double </TD></TR>
   <TR><TD>final_lambda       </TD><TD>1 double </TD></TR>
   <TR><TD>tau_lambda         </TD><TD>1 double </TD></TR>
   <TR><TD>init_S2            </TD><TD>1 double </TD></TR>
   <TR><TD>add_threshold      </TD><TD>1 double </TD></TR>
   </TABLE>
   Then, for each LWPR_SubModel (output dimension):
   <TABLE>
   <TR><TH>Element description</TH><TH>Size of element</TH></TR>
   <TR><TD>"SUBM"             </TD><TD>4 bytes</TD></TR>
   <TR><TD>output dimension   </TD><TD>1 integer</TD></TR>
   <TR><TD>number of RFs      </TD><TD>1 integer</TD></TR>
   <TR><TD>nr. of pruned RFs  </TD><TD>1 integer</TD></TR>
   </TABLE>
   Within a submodel, for each LWPR_ReceptiveField:
   <TABLE>
   <TR><TH>Element description</TH><TH>Size of element</TH></TR>
   <TR><TD>"[RF]"       </TD><TD>4 bytes</TD></TR>
   <TR><TD>nReg         </TD><TD>1 integer</TD></TR>
   <TR><TD>D            </TD><TD>1 nIn*nIn doubles</TD></TR>
   <TR><TD>M            </TD><TD>1 nIn*nIn doubles</TD></TR>
   <TR><TD>alpha        </TD><TD>1 nIn*nIn doubles</TD></TR>
   <TR><TD>beta0        </TD><TD>1 double</TD></TR>
   <TR><TD>beta         </TD><TD>nReg doubles</TD></TR>
   <TR><TD>c            </TD><TD>nIn doubles</TD></TR>
   <TR><TD>SXresYres    </TD><TD>nIn*nReg doubles</TD></TR>
   <TR><TD>SSs2         </TD><TD>nReg doubles</TD></TR>
   <TR><TD>SSYres       </TD><TD>nReg doubles</TD></TR>
   <TR><TD>SSXres       </TD><TD>nIn*nReg doubles</TD></TR>
   <TR><TD>U            </TD><TD>nIn*nReg doubles</TD></TR>
   <TR><TD>P            </TD><TD>nIn*nReg doubles</TD></TR>
   <TR><TD>H            </TD><TD>nReg doubles</TD></TR>
   <TR><TD>r            </TD><TD>nReg doubles</TD></TR>
   <TR><TD>h            </TD><TD>nIn*nIn doubles</TD></TR>
   <TR><TD>b            </TD><TD>nIn*nIn doubles</TD></TR>
   <TR><TD>sum_w        </TD><TD>nReg doubles</TD></TR>
   <TR><TD>sum_e_cv2    </TD><TD>nReg doubles</TD></TR>
   <TR><TD>sum_e2       </TD><TD>1 double</TD></TR>
   <TR><TD>SSp          </TD><TD>1 double</TD></TR>
   <TR><TD>n_data       </TD><TD>nReg doubles</TD></TR>
   <TR><TD>trustworthy  </TD><TD>1 integer</TD></TR>
   <TR><TD>lambda       </TD><TD>nReg doubles</TD></TR>
   <TR><TD>mean_x       </TD><TD>nIn doubles</TD></TR>
   <TR><TD>var_x        </TD><TD>nIn doubles</TD></TR>
   <TR><TD>w            </TD><TD>1 double</TD></TR>
   <TR><TD>s            </TD><TD>nReg doubles</TD></TR>
   </TABLE>   
   As a last sanity check, the file ends with the 4 characters "RPWL". The overall
   structure of a binary LWPR file looks like this:
   
   <b>LWPR</b>...model globals...<b>SUBM</b>...output dim. 0...<b>[RF]</b>...RF 
   0/0...<b>[RF]</b>...RF 0/1...<b>[RF]</b>...RF 0/2...<em>further receptive 
   fields</em>...<b>SUBM</b>...output dim. 1...<b>[RF]</b>...RF 1/0...<b>[RF]</b>...RF 
   1/1...<em>further receptive fields</em>...<b>SUBM</b>...output dim. 2...<b>[RF]</b>...RF 
   2/0...<b>[RF]</b>...RF 2/1...<em>further receptive fields</em>...<em>further 
   sub-models (output dimensions)</em>...<b>RPWL</b>
   
   \ingroup LWPR_C
*/

#ifndef __LWPR_BINIO_H
#define __LWPR_BINIO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Writes an LWPR model to a file 
   \param[in] model    Pointer to a valid LWPR model structure
   \param[in] filename The name of the file
   \return
      - 0 if errors have occured
      - 1 on success
   \ingroup LWPR_C    
*/
int lwpr_write_binary(const LWPR_Model *model, const char *filename);


/** \brief Reads an LWPR model from a binary file 
   \param[in,out] model Pointer to a valid LWPR model structure
   \param[in] filename  Name of the file to read the model from
   \return
      - 0 if errors have occured
      - 1 on success
   \ingroup LWPR_C    
*/
int lwpr_read_binary(LWPR_Model *model, const char *filename);


/** \brief Writes an LWPR model to a file 
   \param[in] model    Pointer to a valid LWPR model structure
   \param[in] fp       Descriptor of an already opened file (see stdio.h)
   \return
      - 0 if errors have occured
      - 1 on success
   \ingroup LWPR_C    
*/
int lwpr_write_binary_fp(const LWPR_Model *model, FILE *fp);

/** \brief Reads an LWPR model from a binary file 
   \param[in,out] model Pointer to a valid LWPR model structure
   \param[in] fp        Descriptor of an already opened file (see stdio.h)
   \return
      - 0 if errors have occured
      - 1 on success
   \ingroup LWPR_C    
*/
int lwpr_read_binary_fp(LWPR_Model *model, FILE *fp);


/** \brief Writes a matrix of doubles into a binary file
   \param[in] fp       File descriptor
   \param[in] M        Number of rows
   \param[in] Ms       Stride parameter (offset between columns)
   \param[in] N        Number of columns
   \param[in] data     Pointer to matrix elements
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_write_matrix(FILE *fp,int M, int Ms, int N, const double *data);

/** \brief Reads a matrix of doubles from a binary file
   \param[in] fp       File descriptor
   \param[in] M        Number of rows
   \param[in] Ms       Stride parameter (offset between columns)
   \param[in] N        Number of columns
   \param[out] data    Pointer to matrix elements
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_read_matrix(FILE *fp, int M, int Ms, int N, double *data);

/** \brief Writes a vector of doubles into a binary file
   \param[in] fp       File descriptor
   \param[in] N        Number of elements
   \param[in] data     Pointer to vector elements
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_write_vector(FILE *fp, int N, const double *data);

/** \brief Reads a vector of doubles from a binary file
   \param[in] fp       File descriptor
   \param[in] N        Number of elements
   \param[out] data     Pointer to vector elements
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_read_vector(FILE *fp, int N, double *data);

/** \brief Writes a scalar (double) into a binary file
   \param[in] fp       File descriptor
   \param[in] data     Scalar value
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_write_scalar(FILE *fp, double data);

/** \brief Reads a scalar (double) from a binary file
   \param[in] fp       File descriptor
   \param[out] data    Pointer to scalar value
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_read_scalar(FILE *fp, double *data);

/** \brief Writes an integer into a binary file
   \param[in] fp       File descriptor
   \param[in] data     Integer value
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_write_int(FILE *fp, int data);

/** \brief Reads an integer from a binary file
   \param[in] fp      File descriptor
   \param[out] data    Pointer to integer
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_read_int(FILE *fp, int *data);

/** \brief Writes a receptive field structure into a binary file
   \param[in] fp     File descriptor
   \param[in] RF     Pointer to a receptive field structure
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_write_rf(FILE *fp, const LWPR_ReceptiveField *RF);

/** \brief Reads a receptive field structure from a binary file
   \param[in] fp      File descriptor
   \param[in,out] sub Pointer to the current LWPR_SubModel, to which a new LWPR_ReceptiveField structure
                      will be added.
   \return
      - 0 if errors have occured
      - 1 on success
*/
int lwpr_io_read_rf(FILE *fp, LWPR_SubModel *sub);


#ifdef __cplusplus
}
#endif

#endif
