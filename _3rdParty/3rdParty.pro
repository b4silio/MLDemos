###########################
# Configuration           #
###########################
TEMPLATE = lib
NAME = 3rdParty
MLPATH =..
CONFIG += mainApp static _3rdParty

include($$MLPATH/MLDemos_variables.pri)

# disable some warnings for 3rd party libraries (not our problem if they do funky stuff in their libraries)
win32-g++|macx|unix{
    QMAKE_CXXFLAGS_WARN_ON = ""
    QMAKE_CXXFLAGS = -Wno-all
    #QMAKE_CXXFLAGS += -Wno-endif-labels
    QMAKE_CXXFLAGS += -Wno-unused-variable
    QMAKE_CXXFLAGS += -Wno-unused-parameter
    #QMAKE_CXXFLAGS += -Wno-switch
    QMAKE_CXXFLAGS += -Wtrigraphs
    QMAKE_CXXFLAGS += -Wreturn-type
    #QMAKE_CXXFLAGS += -Wnon-virtual-dtor
    #QMAKE_CXXFLAGS += -Woverloaded-virtual
    #QMAKE_CXXFLAGS += -Wunused-variable
    #QMAKE_CXXFLAGS += -Wunused-value
    QMAKE_CXXFLAGS += -Wunknown-pragmas
    QMAKE_CXXFLAGS += -Wno-shadow
    QMAKE_CXXFLAGS += -Wno-deprecated-declarations
    QMAKE_CXXFLAGS += -Wno-missing-braces
}

win32{
	DESTDIR = ../_3rdParty
}
###########################
# Files                   #
###########################
HEADERS +=	\
	fgmm/em.h \
	fgmm/fgmm++.hpp \
	fgmm/fgmm.h \
	fgmm/gaussian.h \
	fgmm/regression.h \
	fgmm/smat.h

SOURCES += \
	fgmm/em.cpp \
	fgmm/gaussian.cpp \
	fgmm/gmm.cpp \
	fgmm/gmmregression.cpp \
	fgmm/smat.cpp \
	fgmm/update.cpp

HEADERS +=	\
	MathLib/Differentiator.h \
	MathLib/GradientDescent.h \
	MathLib/IKGroupSolver.h \
	MathLib/IKSubSolver.h \
	MathLib/Macros.h \
	MathLib/MathLib.h \
	MathLib/MathLibCommon.h \
	MathLib/Matrix.h \
	MathLib/Matrix3.h \
	MathLib/Matrix4.h \
	MathLib/ReferenceFrame.h \
	MathLib/Regression.h \
	MathLib/SpatialForce.h \
	MathLib/SpatialFrame.h \
	MathLib/SpatialInertia.h \
	MathLib/SpatialMatrix.h \
	MathLib/SpatialVector.h \
	MathLib/SpatialVelocity.h \
	MathLib/TMatrix.h \
	MathLib/TVector.h \
	MathLib/Vector.h \
	MathLib/Vector3.h

SOURCES += \
	MathLib/Differentiator.cpp \
	MathLib/GradientDescent.cpp \
	MathLib/IKGroupSolver.cpp \
	MathLib/IKSubSolver.cpp \
	MathLib/Macros.cpp \
	MathLib/MathLib.cpp \
	MathLib/MathLibCommon.cpp \
	MathLib/Matrix.cpp \
	MathLib/Matrix3.cpp \
	MathLib/Matrix4.cpp \
	MathLib/ReferenceFrame.cpp \
	MathLib/Regression.cpp \
	MathLib/SpatialForce.cpp \
	MathLib/SpatialFrame.cpp \
	MathLib/SpatialInertia.cpp \
	MathLib/SpatialMatrix.cpp \
	MathLib/SpatialVector.cpp \
	MathLib/SpatialVelocity.cpp \
	MathLib/TMatrix.cpp \
	MathLib/TVector.cpp \
	MathLib/Vector.cpp \
	MathLib/Vector3.cpp

HEADERS += \
	PSO/pso.h \
	PSO/memoryAllocation.h \
	PSO/optimizer.h

SOURCES += \
	PSO/pso.cpp \
	PSO/memoryAllocation.cpp \
	PSO/optimizer.cpp

HEADERS +=	\
	newmat11/controlw.h \
	newmat11/include.h \
	newmat11/myexcept.h \
	newmat11/newmat.h \
	newmat11/newmatap.h \
	newmat11/newmatio.h \
	newmat11/newmatnl.h \
	newmat11/newmatrc.h \
	newmat11/newmatrm.h \
	newmat11/precisio.h \
	newmat11/solution.h

SOURCES += \
	newmat11/bandmat.cpp \
	newmat11/cholesky.cpp \
	newmat11/evalue.cpp \
	newmat11/fft.cpp \
	newmat11/hholder.cpp \
	newmat11/jacobi.cpp \
	newmat11/myexcept.cpp \
	newmat11/newfft.cpp \
	newmat11/newmat1.cpp \
	newmat11/newmat2.cpp \
	newmat11/newmat3.cpp \
	newmat11/newmat4.cpp \
	newmat11/newmat5.cpp \
	newmat11/newmat6.cpp \
	newmat11/newmat7.cpp \
	newmat11/newmat8.cpp \
	newmat11/newmat9.cpp \
	newmat11/newmatex.cpp \
	newmat11/newmatnl.cpp \
	newmat11/newmatrm.cpp \
	newmat11/nm_misc.cpp \
	newmat11/solution.cpp \
	newmat11/sort.cpp \
	newmat11/submat.cpp \
	newmat11/svd.cpp

HEADERS += \
	lwpr/lwpr.h \
	lwpr/lwpr.hh \
	lwpr/lwpr_aux.h \
	lwpr/lwpr_binio.h \
	lwpr/lwpr_config.h \
	lwpr/lwpr_config_def.h \
	lwpr/lwpr_math.h \
	lwpr/lwpr_mem.h \
	lwpr/lwpr_xml.h

SOURCES +=  \
	lwpr/lwpr.c \
	lwpr/lwpr_aux.c \
	lwpr/lwpr_binio.c \
	lwpr/lwpr_math.c \
	lwpr/lwpr_mem.c \
	lwpr/lwpr_xml.c

HEADERS += 	JnS/Matutil.h \
			JnS/JnS.h
SOURCES += 	JnS/Matutil.cpp \
			JnS/JnS.cpp
HEADERS += 	liblinear/linear.h \
			liblinear/tron.h \
			liblinear/blasp.h \
			liblinear/blas.h
SOURCES += 	liblinear/linear.cpp \
			liblinear/tron.cpp \
			liblinear/daxpy.c \
			liblinear/ddot.c \
			liblinear/dnrm2.c \
			liblinear/dscal.c

HEADERS += ANN/ANN.h \
	ANN/ANNperf.h \
	ANN/ANNx.h \
	ANN/bd_tree.h \
	ANN/kd_fix_rad_search.h \
	ANN/kd_pr_search.h \
	ANN/kd_search.h \
	ANN/kd_split.h \
	ANN/kd_tree.h \
	ANN/kd_util.h \
	ANN/pr_queue.h \
	ANN/pr_queue_k.h

SOURCES += ANN/ANN.cpp \
	ANN/bd_fix_rad_search.cpp \
	ANN/bd_pr_search.cpp \
	ANN/bd_search.cpp \
	ANN/bd_tree.cpp \
	ANN/brute.cpp \
	ANN/kd_dump.cpp \
	ANN/kd_fix_rad_search.cpp \
	ANN/kd_pr_search.cpp \
	ANN/kd_search.cpp \
	ANN/kd_split.cpp \
	ANN/kd_tree.cpp \
	ANN/kd_util.cpp \
	ANN/perf.cpp

HEADERS +=	\
	dlib/algs.h \
	dlib/array.h \
	dlib/array2d.h \
	dlib/assert.h \
	dlib/base64.h \
	dlib/bayes_utils.h \
	dlib/bigint.h \
	dlib/binary_search_tree.h \
	dlib/bit_stream.h \
	dlib/bound_function_pointer.h \
	dlib/byte_orderer.h \
	dlib/cmd_line_parser.h \
	dlib/compress_stream.h \
	dlib/conditioning_class.h \
	dlib/config_reader.h \
	dlib/cpp_pretty_printer.h \
	dlib/cpp_tokenizer.h \
	dlib/crc32.h \
	dlib/data_io.h \
	dlib/dir_nav.h \
	dlib/directed_graph.h \
	dlib/enable_if.h \
	dlib/entropy_decoder.h \
	dlib/entropy_decoder_model.h \
	dlib/entropy_encoder.h \
	dlib/entropy_encoder_model.h \
	dlib/error.h \
	dlib/geometry.h \
	dlib/graph.h \
	dlib/graph_utils.h \
	dlib/gui_core.h \
	dlib/gui_widgets.h \
	dlib/hash_map.h \
	dlib/hash_set.h \
	dlib/hash_table.h \
	dlib/image_io.h \
	dlib/image_keypoint.h \
	dlib/image_transforms.h \
	dlib/is_kind.h \
	dlib/linker.h \
	dlib/logger.h \
	dlib/lz77_buffer.h \
	dlib/lzp_buffer.h \
	dlib/manifold_regularization.h \
	dlib/map.h \
	dlib/matrix.h \
	dlib/md5.h \
	dlib/member_function_pointer.h \
	dlib/memory_manager.h \
	dlib/memory_manager_global.h \
	dlib/memory_manager_stateless.h \
	dlib/misc_api.h \
	dlib/mlp.h \
	dlib/noncopyable.h \
	dlib/opencv.h \
	dlib/optimization.h \
	dlib/pipe.h \
	dlib/pixel.h \
	dlib/platform.h \
	dlib/quantum_computing.h \
	dlib/queue.h \
	dlib/rand.h \
	dlib/ref.h \
	dlib/reference_counter.h \
	dlib/revision.h \
	dlib/sequence.h \
	dlib/serialize.h \
	dlib/server.h \
	dlib/set.h \
	dlib/set_utils.h \
	dlib/sliding_buffer.h \
	dlib/smart_pointers.h \
	dlib/smart_pointers_thread_safe.h \
	dlib/sockets.h \
	dlib/sockstreambuf.h \
	dlib/sort.h \
	dlib/stack.h \
	dlib/stack_trace.h \
	dlib/static_map.h \
	dlib/static_set.h \
	dlib/statistics.h \
	dlib/std_allocator.h \
	dlib/stl_checked.h \
	dlib/string.h \
	dlib/svm.h \
	dlib/svm_threaded.h \
	dlib/sync_extension.h \
	dlib/threads.h \
	dlib/time_this.h \
	dlib/timeout.h \
	dlib/timer.h \
	dlib/tokenizer.h \
	dlib/tuple.h \
	dlib/type_safe_union.h \
	dlib/uintn.h \
	dlib/unicode.h \
	dlib/windows_magic.h \
	dlib/xml_parser.h \

HEADERS +=	\
	nlopt.hpp \
	nlopt/tools.h \
	nlopt/stogo_config.h \
	nlopt/stogo.h \
	nlopt/soboldata.h \
	nlopt/slsqp.h \
	nlopt/redblack.h \
	nlopt/praxis.h \
	nlopt/nlopt-util.h \
	nlopt/nlopt-internal.h \
	nlopt/nlopt-in.hpp \
	nlopt/nlopt_optimize_usage.h \
	nlopt/nlopt.hpp \
	nlopt/nlopt.h \
	nlopt/newuoa.h \
	nlopt/neldermead.h \
	nlopt/mma.h \
	nlopt/mlsl.h \
	nlopt/luksan.h \
	nlopt/local.h \
	nlopt/linalg.h \
	nlopt/isres.h \
	nlopt/global.h \
	nlopt/f77funcs_.h \
	nlopt/f77funcs.h \
	nlopt/direct-internal.h \
	nlopt/direct.h \
	nlopt/crs.h \
	nlopt/cobyla.h \
	nlopt/cdirect.h \
	nlopt/bobyqa.h \
	nlopt/auglag.h \
	nlopt/config.h

SOURCES += 	\
	nlopt/tools.cc \
	nlopt/timer.c \
	nlopt/stop.c \
	nlopt/stogo.cc \
	nlopt/sobolseq.c \
	nlopt/slsqp.c \
	nlopt/sbplx.c \
	nlopt/rescale.c \
	nlopt/redblack.c \
	nlopt/qsort_r.c \
	nlopt/pssubs.c \
	nlopt/praxis.c \
	nlopt/pnet.c \
	nlopt/plis.c \
	nlopt/plip.c \
	nlopt/options.c \
	nlopt/optimize.c \
	nlopt/nldrmd.c \
	nlopt/newuoa.c \
	nlopt/mt19937ar.c \
	nlopt/mssubs.c \
	nlopt/mma.c \
	nlopt/mlsl.c \
	nlopt/local.cc \
	nlopt/linalg.cc \
	nlopt/isres.c \
	nlopt/hybrid.c \
	nlopt/global.cc \
	nlopt/general.c \
	nlopt/f77api.c \
	nlopt/DIRsubrout.c \
	nlopt/DIRserial.c \
	nlopt/direct_wrap.c \
	nlopt/DIRect.c \
	nlopt/deprecated.c \
	nlopt/crs.c \
	nlopt/cobyla.c \
	nlopt/cdirect.c \
	nlopt/bobyqa.c \
	nlopt/auglag.c

HEADERS += \
	LAMP_HMM/discreteObsProb.h \
	LAMP_HMM/explicitDurationTrans.h \
	LAMP_HMM/gammaProb.h \
	LAMP_HMM/gaussianObsProb.h \
	LAMP_HMM/hmm.h \
	LAMP_HMM/initStateProb.h \
	LAMP_HMM/obs.h \
	LAMP_HMM/obsProb.h \
	LAMP_HMM/obsSeq.h \
	LAMP_HMM/plainStateTrans.h \
	LAMP_HMM/stateTrans.h \
	LAMP_HMM/utils.h \
	LAMP_HMM/vectorObsProb.h

SOURCES += \
	LAMP_HMM/checkDurationDist.cpp \
	LAMP_HMM/discreteObsProb.cpp \
	LAMP_HMM/explicitDurationTrans.cpp \
	LAMP_HMM/gammaProb.cpp \
	LAMP_HMM/gaussianObsProb.cpp \
	LAMP_HMM/hmm.cpp \
	LAMP_HMM/hmmFind.cpp \
	LAMP_HMM/initStateProb.cpp \
	LAMP_HMM/obsSeq.cpp \
	LAMP_HMM/plainStateTrans.cpp \
	LAMP_HMM/readConfigFile.cpp \
	LAMP_HMM/utils.cpp \
	LAMP_HMM/vectorObsProb.cpp

HEADERS += \
        gsl/config.h \
    gsl/ieee-utils/gsl_ieee_utils.h \
    gsl/linalg/tridiag.h

SOURCES += \
    gsl/block/gsl_block_init.c \
    gsl/block/gsl_block_file.c \
    gsl/block/gsl_block_block.c \
    gsl/cblas/gsl_cblas_strsv.c \
    gsl/cblas/gsl_cblas_strsm.c \
    gsl/cblas/gsl_cblas_strmv.c \
    gsl/cblas/gsl_cblas_strmm.c \
    gsl/cblas/gsl_cblas_stpsv.c \
    gsl/cblas/gsl_cblas_stpmv.c \
    gsl/cblas/gsl_cblas_stbsv.c \
    gsl/cblas/gsl_cblas_stbmv.c \
    gsl/cblas/gsl_cblas_ssyrk.c \
    gsl/cblas/gsl_cblas_ssyr2k.c \
    gsl/cblas/gsl_cblas_ssyr2.c \
    gsl/cblas/gsl_cblas_ssyr.c \
    gsl/cblas/gsl_cblas_ssymv.c \
    gsl/cblas/gsl_cblas_ssymm.c \
    gsl/cblas/gsl_cblas_sswap.c \
    gsl/cblas/gsl_cblas_sspr2.c \
    gsl/cblas/gsl_cblas_sspr.c \
    gsl/cblas/gsl_cblas_sspmv.c \
    gsl/cblas/gsl_cblas_sscal.c \
    gsl/cblas/gsl_cblas_ssbmv.c \
    gsl/cblas/gsl_cblas_srotmg.c \
    gsl/cblas/gsl_cblas_srotm.c \
    gsl/cblas/gsl_cblas_srotg.c \
    gsl/cblas/gsl_cblas_srot.c \
    gsl/cblas/gsl_cblas_snrm2.c \
    gsl/cblas/gsl_cblas_sger.c \
    gsl/cblas/gsl_cblas_sgemv.c \
    gsl/cblas/gsl_cblas_sgemm.c \
    gsl/cblas/gsl_cblas_sgbmv.c \
    gsl/cblas/gsl_cblas_sdsdot.c \
    gsl/cblas/gsl_cblas_sdot.c \
    gsl/cblas/gsl_cblas_scopy.c \
    gsl/cblas/gsl_cblas_scnrm2.c \
    gsl/cblas/gsl_cblas_scasum.c \
    gsl/cblas/gsl_cblas_saxpy.c \
    gsl/cblas/gsl_cblas_sasum.c \
    gsl/cblas/gsl_cblas_izamax.c \
    gsl/cblas/gsl_cblas_isamax.c \
    gsl/cblas/gsl_cblas_idamax.c \
    gsl/cblas/gsl_cblas_icamax.c \
    gsl/cblas/gsl_cblas_dznrm2.c \
    gsl/cblas/gsl_cblas_dzasum.c \
    gsl/cblas/gsl_cblas_dtrsv.c \
    gsl/cblas/gsl_cblas_dtrsm.c \
    gsl/cblas/gsl_cblas_dtrmv.c \
    gsl/cblas/gsl_cblas_dtrmm.c \
    gsl/cblas/gsl_cblas_dtpsv.c \
    gsl/cblas/gsl_cblas_dtpmv.c \
    gsl/cblas/gsl_cblas_dtbsv.c \
    gsl/cblas/gsl_cblas_dtbmv.c \
    gsl/cblas/gsl_cblas_dsyrk.c \
    gsl/cblas/gsl_cblas_dsyr2k.c \
    gsl/cblas/gsl_cblas_dsyr2.c \
    gsl/cblas/gsl_cblas_dsyr.c \
    gsl/cblas/gsl_cblas_dsymv.c \
    gsl/cblas/gsl_cblas_dsymm.c \
    gsl/cblas/gsl_cblas_dswap.c \
    gsl/cblas/gsl_cblas_dspr2.c \
    gsl/cblas/gsl_cblas_dspr.c \
    gsl/cblas/gsl_cblas_dspmv.c \
    gsl/cblas/gsl_cblas_dsdot.c \
    gsl/cblas/gsl_cblas_dscal.c \
    gsl/cblas/gsl_cblas_dsbmv.c \
    gsl/cblas/gsl_cblas_drotmg.c \
    gsl/cblas/gsl_cblas_drotm.c \
    gsl/cblas/gsl_cblas_drotg.c \
    gsl/cblas/gsl_cblas_drot.c \
    gsl/cblas/gsl_cblas_dnrm2.c \
    gsl/cblas/gsl_cblas_dger.c \
    gsl/cblas/gsl_cblas_dgemv.c \
    gsl/cblas/gsl_cblas_dgemm.c \
    gsl/cblas/gsl_cblas_dgbmv.c \
    gsl/cblas/gsl_cblas_ddot.c \
    gsl/cblas/gsl_cblas_dcopy.c \
    gsl/cblas/gsl_cblas_daxpy.c \
    gsl/cblas/gsl_cblas_dasum.c \
    gsl/cblas/gsl_cblas_ctrsv.c \
    gsl/cblas/gsl_cblas_ctrsm.c \
    gsl/cblas/gsl_cblas_ctrmv.c \
    gsl/cblas/gsl_cblas_ctrmm.c \
    gsl/cblas/gsl_cblas_ctpsv.c \
    gsl/cblas/gsl_cblas_ctpmv.c \
    gsl/cblas/gsl_cblas_ctbsv.c \
    gsl/cblas/gsl_cblas_ctbmv.c \
    gsl/cblas/gsl_cblas_csyrk.c \
    gsl/cblas/gsl_cblas_csyr2k.c \
    gsl/cblas/gsl_cblas_csymm.c \
    gsl/cblas/gsl_cblas_cswap.c \
    gsl/cblas/gsl_cblas_csscal.c \
    gsl/cblas/gsl_cblas_cscal.c \
    gsl/cblas/gsl_cblas_chpr2.c \
    gsl/cblas/gsl_cblas_chpr.c \
    gsl/cblas/gsl_cblas_chpmv.c \
    gsl/cblas/gsl_cblas_cherk.c \
    gsl/cblas/gsl_cblas_cher2k.c \
    gsl/cblas/gsl_cblas_cher2.c \
    gsl/cblas/gsl_cblas_cher.c \
    gsl/cblas/gsl_cblas_chemv.c \
    gsl/cblas/gsl_cblas_chemm.c \
    gsl/cblas/gsl_cblas_chbmv.c \
    gsl/cblas/gsl_cblas_cgeru.c \
    gsl/cblas/gsl_cblas_cgerc.c \
    gsl/cblas/gsl_cblas_cgemv.c \
    gsl/cblas/gsl_cblas_cgemm.c \
    gsl/cblas/gsl_cblas_cgbmv.c \
    gsl/cblas/gsl_cblas_cdotu_sub.c \
    gsl/cblas/gsl_cblas_cdotc_sub.c \
    gsl/cblas/gsl_cblas_ccopy.c \
    gsl/cblas/gsl_cblas_caxpy.c \
    gsl/cblas/gsl_cblas_ztrsv.c \
    gsl/cblas/gsl_cblas_ztrsm.c \
    gsl/cblas/gsl_cblas_ztrmv.c \
    gsl/cblas/gsl_cblas_ztrmm.c \
    gsl/cblas/gsl_cblas_ztpsv.c \
    gsl/cblas/gsl_cblas_ztpmv.c \
    gsl/cblas/gsl_cblas_ztbsv.c \
    gsl/cblas/gsl_cblas_ztbmv.c \
    gsl/cblas/gsl_cblas_zsyrk.c \
    gsl/cblas/gsl_cblas_zsyr2k.c \
    gsl/cblas/gsl_cblas_zsymm.c \
    gsl/cblas/gsl_cblas_zswap.c \
    gsl/cblas/gsl_cblas_zscal.c \
    gsl/cblas/gsl_cblas_zhpr2.c \
    gsl/cblas/gsl_cblas_zhpr.c \
    gsl/cblas/gsl_cblas_zhpmv.c \
    gsl/cblas/gsl_cblas_zherk.c \
    gsl/cblas/gsl_cblas_zher2k.c \
    gsl/cblas/gsl_cblas_zher2.c \
    gsl/cblas/gsl_cblas_zher.c \
    gsl/cblas/gsl_cblas_zhemv.c \
    gsl/cblas/gsl_cblas_zhemm.c \
    gsl/cblas/gsl_cblas_zhbmv.c \
    gsl/cblas/gsl_cblas_zgeru.c \
    gsl/cblas/gsl_cblas_zgerc.c \
    gsl/cblas/gsl_cblas_zgemv.c \
    gsl/cblas/gsl_cblas_zgemm.c \
    gsl/cblas/gsl_cblas_zgbmv.c \
    gsl/cblas/gsl_cblas_zdscal.c \
    gsl/cblas/gsl_cblas_zdotu_sub.c \
    gsl/cblas/gsl_cblas_zdotc_sub.c \
    gsl/cblas/gsl_cblas_zcopy.c \
    gsl/cblas/gsl_cblas_zaxpy.c \
    gsl/cblas/gsl_cblas_xerbla.c \
    gsl/complex/gsl_complex_math.c \
    gsl/complex/gsl_complex_inline.c \
    gsl/ieee-utils/gsl_ieee-utils_read.c \
    gsl/ieee-utils/gsl_ieee-utils_print.c \
    gsl/ieee-utils/gsl_ieee-utils_make_rep.c \
    gsl/ieee-utils/gsl_ieee-utils_fp.c \
    gsl/ieee-utils/gsl_ieee-utils_env.c \
    gsl/linalg/gsl_linalg_tridiag.c \
    gsl/linalg/gsl_linalg_symmtd.c \
    gsl/linalg/gsl_linalg_svd.c \
    gsl/linalg/gsl_linalg_qrpt.c \
    gsl/linalg/gsl_linalg_qr.c \
    gsl/linalg/gsl_linalg_ptlq.c \
    gsl/linalg/gsl_linalg_multiply.c \
    gsl/linalg/gsl_linalg_luc.c \
    gsl/linalg/gsl_linalg_lu.c \
    gsl/linalg/gsl_linalg_lq.c \
    gsl/linalg/gsl_linalg_householdercomplex.c \
    gsl/linalg/gsl_linalg_householder.c \
    gsl/linalg/gsl_linalg_hh.c \
    gsl/linalg/gsl_linalg_hesstri.c \
    gsl/linalg/gsl_linalg_hessenberg.c \
    gsl/linalg/gsl_linalg_hermtd.c \
    gsl/linalg/gsl_linalg_exponential.c \
    gsl/linalg/gsl_linalg_choleskyc.c \
    gsl/linalg/gsl_linalg_cholesky.c \
    gsl/linalg/gsl_linalg_bidiag.c \
    gsl/linalg/gsl_linalg_balancemat.c \
    gsl/linalg/gsl_linalg_balance.c \
    gsl/matrix/gsl_matrix_view.c \
    gsl/matrix/gsl_matrix_swap.c \
    gsl/matrix/gsl_matrix_submatrix.c \
    gsl/matrix/gsl_matrix_rowcol.c \
    gsl/matrix/gsl_matrix_prop.c \
    gsl/matrix/gsl_matrix_oper.c \
    gsl/matrix/gsl_matrix_minmax.c \
    gsl/matrix/gsl_matrix_matrix.c \
    gsl/matrix/gsl_matrix_init.c \
    gsl/matrix/gsl_matrix_getset.c \
    gsl/matrix/gsl_matrix_file.c \
    gsl/matrix/gsl_matrix_copy.c \
    gsl/multifit/gsl_multifit_work.c \
    gsl/multifit/gsl_multifit_multilinear.c \
    gsl/multifit/gsl_multifit_lmder.c \
    gsl/multifit/gsl_multifit_gradient.c \
    gsl/multifit/gsl_multifit_fsolver.c \
    gsl/multifit/gsl_multifit_fdfsolver.c \
    gsl/multifit/gsl_multifit_covar.c \
    gsl/multifit/gsl_multifit_convergence.c \
    gsl/permutation/gsl_permutation_permute.c \
    gsl/permutation/gsl_permutation_permutation.c \
    gsl/permutation/gsl_permutation_inline.c \
    gsl/permutation/gsl_permutation_init.c \
    gsl/permutation/gsl_permutation_file.c \
    gsl/permutation/gsl_permutation_canonical.c \
    gsl/sys/gsl_sys_prec.c \
    gsl/sys/gsl_sys_pow_int.c \
    gsl/sys/gsl_sys_minmax.c \
    gsl/sys/gsl_sys_log1p.c \
    gsl/sys/gsl_sys_ldfrexp.c \
    gsl/sys/gsl_sys_invhyp.c \
    gsl/sys/gsl_sys_infnan.c \
    gsl/sys/gsl_sys_hypot.c \
    gsl/sys/gsl_sys_fdiv.c \
    gsl/sys/gsl_sys_fcmp.c \
    gsl/sys/gsl_sys_expm1.c \
    gsl/sys/gsl_sys_coerce.c \
    gsl/test/gsl_test_results.c \
    gsl/vector/gsl_vector_view.c \
    gsl/vector/gsl_vector_vector.c \
    gsl/vector/gsl_vector_swap.c \
    gsl/vector/gsl_vector_subvector.c \
    gsl/vector/gsl_vector_reim.c \
    gsl/vector/gsl_vector_prop.c \
    gsl/vector/gsl_vector_oper.c \
    gsl/vector/gsl_vector_minmax.c \
    gsl/vector/gsl_vector_init.c \
    gsl/vector/gsl_vector_file.c \
    gsl/vector/gsl_vector_copy.c \
    gsl/err/gsl_err_strerror.c \
    gsl/err/gsl_err_stream.c \
    gsl/err/gsl_err_message.c \
    gsl/err/gsl_err_error.c \
    gsl/sort/gsl_sort_subsetind.c \
    gsl/sort/gsl_sort_subset.c \
    gsl/sort/gsl_sort_sortvecind.c \
    gsl/sort/gsl_sort_sortvec.c \
    gsl/sort/gsl_sort_sortind.c \
    gsl/sort/gsl_sort_sort.c \
    gsl/blas/gsl_blas_blas.c

HEADERS += \
    matio/mat5.h \
    matio/matio.h \
    matio/mat4.h \
    matio/mat73.h \
    matio/matioConfig.h \
    matio/matio_pubconf.h \
    matio/matio_private.h

SOURCES += \
    matio/inflate.c \
    matio/mat4.c \
    matio/read_data.c \
    matio/io.c \
    matio/mat73.c \
    matio/matvar_cell.c \
    matio/snprintf.c \
    matio/endian.c \
    matio/mat.c \
    matio/mat5.c \
    matio/matvar_struct.c

HEADERS += \
    jacgrid/atom.h \
    jacgrid/cell_table.h \
    jacgrid/grid.h \
    jacgrid/jacgrid.h \
    jacgrid/jacgrid_private.h \
    jacgrid/linalg.h \
    jacgrid/plane.h \
    jacgrid/surface.h

SOURCES += \
    jacgrid/atom.cpp \
    jacgrid/grid.cpp \
    jacgrid/isosurface.cpp \
    jacgrid/plane.cpp \
    jacgrid/surface.cpp

#unix{
#	BOOST = /usr/local/boost_1_47_0
#}else{
#	BOOST = E:/DEV/boost_1_47_0
#}
#INCLUDEPATH += $$BOOST

#HEADERS += \
#	HMMlib/allocator_traits.hpp \
#	HMMlib/float_traits.hpp \
#	HMMlib/hmm_matrix.hpp \
#	HMMlib/hmm_table.hpp \
#	HMMlib/hmm_vector.hpp \
#	HMMlib/hmm.hpp \
#	HMMlib/operator_traits.hpp \
#	HMMlib/sse_operator_traits.hpp

#SOURCES += \
#	HMMlib/hmm_matrix_test.cpp \
#	HMMlib/hmm_test.cpp \
#	HMMlib/hmm_vector_test.cpp
