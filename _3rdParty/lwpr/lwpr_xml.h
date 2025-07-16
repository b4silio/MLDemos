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


/** \file lwpr_xml.h
   \brief Prototypes for XML related LWPR subroutines
   
   This header file describes routines for writing LWPR models to XML files, and
   parsing LWPR models from XML files, if EXPAT is available.
   
   An XML document type definition (DTD) is provided in the file "include/lwpr_xml.dtd".
   \ingroup LWPR_C       
*/

#ifndef __LWPR_XML_H
#define __LWPR_XML_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


/** \brief Data structure used for parsing an LWPR model from an XML file. */
typedef struct {
   int level;        /**< \brief Parser level: 0 = global, 1 = Model, 2 = SubModel, 3 = ReceptiveField */
   int numSub;       /**< \brief Number of SubModels */
   int curSubNumRF;  /**< \brief Number of ReceptiveFields in current SubModel */
   int curSub;       /**< \brief Currently parsed SubModel */
   int curRF;        /**< \brief Currently parsed ReceptiveField */
   int curType;      /**< \brief Expected data type: 0 = struct, 1 = int, 2 = scalar, 3 = vector, 4 = matrix */
   void *curPtr;     /**< \brief Pointer to the data element that has to be filled */
   int M;            /**< \brief Number of rows of current data element */
   int MS;           /**< \brief Offset between columns of current data element */
   int N;            /**< \brief Number of columns of current data element */
   int readM;        /**< \brief Number of already read rows of current data element */
   int readN;        /**< \brief Number of already read columns of current data element */
   int numErrors;    /**< \brief Number of errors encountered during parsing */
   int numWarnings;  /**< \brief Number of warnings encountered during parsing */
   FILE *errFile;    /**< \brief stdio-file to write errors and warnings to, must be open. If this is NULL, errors and warnings are not reported. */
   LWPR_Model *model;/**< \brief Pointer to the LWPR_Model structure that is to be filled */
} LWPR_ParserData;

/** \brief Writes an LWPR model to an XML file 
   \param[in] model    Pointer to a valid LWPR model structure
   \param[in] filename Name of the XML file to write the model to
   \return
      - 0 in case of failure (e.g. file could not be opened for writing)
      - 1 in case of success
   \ingroup LWPR_C          
*/
int lwpr_write_xml(const LWPR_Model *model, const char *filename);

/** \brief Writes an LWPR model to an XML file 
   \param[in] model    Pointer to a valid LWPR model structure
   \param[in] fp       Descriptor of an already opened file (see stdio.h)
   \ingroup LWPR_C    
*/
void lwpr_write_xml_fp(const LWPR_Model *model,FILE *fp);

/** \brief Parse an LWPR model from an XML file 
   
   \param[in]  model        Pointer to a valid LWPR model structure
   \param[in]  filename     Name of the XML file
   \param[out] numWarnings  Number of warnings encountered during parsing
   \return
      The number of errors encountered during parsing, which is >= 10000 if
      the XML structure itself is invalid (as detected by EXPAT).
      An example of other errors (each counted as 1) is an invalid 
      number of elements in vectors and matrices.
      If the file could not be opened for reading, the function 
      returns -1.

   If the library has been compiled without EXPAT support, this
   function is just a dummy and returns -2.
   \ingroup LWPR_C       
*/
int lwpr_read_xml(LWPR_Model *model, const char *filename, int *numWarnings);

/** \brief Writes a matrix as an XML tag into a file
   \param[in] fp       File descriptor
   \param[in] level    Indicates global (0), model (1), submodel (2) or receptive field (3)
   \param[in] name     Name of the matrix
   \param[in] M        Number of rows
   \param[in] Ms       Stride parameter (offset between adjacent columns)
   \param[in] N        Number of columns
   \param[in] val      Pointer to the elements of the matrix 
*/
void lwpr_xml_write_matrix(FILE *fp, int level, const char *name, 
      int M, int Ms, int N, const double *val);

/** \brief Writes a vector as an XML tag into a file
   \param[in] fp       File descriptor
   \param[in] level    Indicates global (0), model (1), submodel (2) or receptive field (3)
   \param[in] name     Name of the vector
   \param[in] N        Number of elements
   \param[in] val      Pointer to the elements of the matrix 
*/
void lwpr_xml_write_vector(FILE *fp, int level, const char *name, 
      int N, const double *val);

/** \brief Writes an interger as an XML tag into a file
   \param[in] fp       File descriptor
   \param[in] level    Indicates global (0), model (1), submodel (2) or receptive field (3)
   \param[in] name     Name of the variable
   \param[in] val      Integer value
*/
void lwpr_xml_write_int(FILE *fp, int level, const char *name, int val);

/** \brief Writes a scalar (double) as an XML tag into a file
   \param[in] fp       File descriptor
   \param[in] level    Indicates global (0), model (1), submodel (2) or receptive field (3)
   \param[in] name     Name of the variable
   \param[in] val      Scalar value
*/
void lwpr_xml_write_scalar(FILE *fp, int level, const char *name, double val);

/** \brief Writes a receptive field structure as XML tags into a file
   \param[in] fp     File descriptor
   \param[in] RF     Pointer to a receptive field structure
*/
void lwpr_xml_write_rf(FILE *fp, const LWPR_ReceptiveField *RF);

/** \brief Checks whether an XML tag describes a valid scalar (int or double)
   \param[in] atts         Array of XML attributes
   \param[out] fieldName   The name of the variable, or NULL if unsuccessful
   \return 
      - 1 in case of success
      - 0 in case of failure
*/
int lwpr_xml_parse_scalar(const char **atts, const char **fieldName);

/** \brief Checks whether an XML tag describes a valid vector
   \param[in] atts         Array of XML attributes
   \param[out] fieldName   The name of the vector, or NULL if unsuccessful
   \param[out] N           Number of elements of the vector
   \return 
      - 1 in case of success
      - 0 in case of failure
*/
int lwpr_xml_parse_vector(const char **atts, const char **fieldName, int *N);

/** \brief Checks whether an XML tag describes a valid matrix
   \param[in] atts         Array of XML attributes
   \param[out] fieldName   The name of the matrix, or NULL if unsuccessful
   \param[out] M           Number of rows of the matrix   
   \param[out] N           Number of columns of the matrix
   \return 
      - 1 in case of success
      - 0 in case of failure
*/
int lwpr_xml_parse_matrix(const char **atts, const char **fieldName, int *M, int *N);

/** \brief Auxiliary routine to report a parsing error 
   \param[in] ud  Pointer to parser data structure (including LWPR model etc.)
   \param[in] msg Error message
*/
void lwpr_xml_error(LWPR_ParserData *ud, const char *msg);

/** \brief Auxiliary routine to report a "bad dimensionality" parsing error 
   \param[in] ud  Pointer to parser data structure (including LWPR model etc.)
   \param[in] fieldname    Name of variable where error occured
   \param[in] wishM        Number of desired rows, or 1 in case of scalars / vectors
   \param[in] wishN        Number of desired columns, or elements in case of vectors
*/
void lwpr_xml_dim_error(LWPR_ParserData *ud, const char *fieldname,int wishM,int wishN);

/** \brief Auxiliary routine to report an "unknown element" warning
   \param[in] ud  Pointer to parser data structure (including LWPR model etc.)
   \param[in] fieldname    Name of variable 
*/
void lwpr_xml_report_unknown(LWPR_ParserData *ud, const char *fieldname);

/** \brief Callback for EXPAT parser, start of a new element */
void lwpr_xml_start_element(void *userData, const char *name, const char **atts);
/** \brief Callback for EXPAT parser, element finished */
void lwpr_xml_end_element(void *userData, const char *name);
/** \brief Callback for EXPAT parser, for data between enclosing tags */
void lwpr_xml_handle_data(void *userData, const char *s, int len);

/** \brief Reads a file into memory
   \param[in]  filename    Name of the XML file
   \param[out] buffer      On return, the pointer to the contents of the file
   \return
      The number of bytes read from the file, or -1 in case of failure.
   
   Note that the allocated memory has to be disposed using LWPR_FREE() afterwards.
*/      
int lwpr_xml_read_file_into_buffer(const char *filename, char **buffer);

#ifdef __cplusplus
}
#endif

#endif
