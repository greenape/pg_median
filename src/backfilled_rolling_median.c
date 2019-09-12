#include "postgres.h"
#include "fmgr.h"
#include "windowapi.h"
#include "quick_select.h"

typedef struct {
    double first_median;
} backfilled_rolling_median_context;

PG_FUNCTION_INFO_V1(backfilled_rolling_median);

Datum backfilled_rolling_median(PG_FUNCTION_ARGS) {
    WindowObject win_obj = PG_WINDOW_OBJECT();
    uint32_t row = WinGetCurrentPosition(win_obj);
    uint32_t j;
    double *med;
    double median;
    Datum value;
    bool isnull, isout;
    bool window_size_is_null;
    backfilled_rolling_median_context *context;
    Datum window_size_datum = WinGetFuncArgCurrent(win_obj, 1, &window_size_is_null);
    uint32_t window_size = DatumGetInt32(window_size_datum);
    if (row == 0) {
        /* Validate input parameters */
        if (window_size_is_null || window_size % 2 == 0) {
            ereport(ERROR,
                    (
                            errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                                    errmsg("window size must be odd"),
                                    errdetail("value %d is even or null", window_size),
                                    errhint("make it odd")
                    )
            );
            PG_RETURN_NULL();
        }

    }

    if(window_size > WinGetPartitionRowCount(win_obj)) {
        PG_RETURN_NULL();
    }



    context = (backfilled_rolling_median_context *) WinGetPartitionLocalMemory(win_obj, sizeof(backfilled_rolling_median_context));
    if (row == 0) {
        /* Need to calculate for first viable median so we can backfill. */
        med = palloc(window_size * sizeof(double));
        for (j = 0; j < window_size; j++) {
            value = WinGetFuncArgInPartition(win_obj, 0, j,
                                             WINDOW_SEEK_HEAD, false, &isnull, &isout);
            med[j] = DatumGetFloat8(value);

            if(isnull) {
                PG_RETURN_NULL();
            }
        }

        context->first_median = quick_select(med, window_size);
        pfree(med);
    }

    if (row > (window_size-1)) {
        med = palloc(window_size * sizeof(double));
        for (j = 0; j < window_size; j++) {
            value = WinGetFuncArgInPartition(win_obj, 0, row - j,
                                             WINDOW_SEEK_HEAD, false, &isnull, &isout);
            med[j] = DatumGetFloat8(value);
            if(isnull) {
                PG_RETURN_NULL();
            }
        }

        median = quick_select(med, window_size);
        pfree(med);
        PG_RETURN_FLOAT8(median);
    } else {
        PG_RETURN_FLOAT8(context->first_median);
    }
}