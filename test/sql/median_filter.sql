SELECT median_filter(v::double precision, 5) over() FROM generate_series(1, 10) as t(v);
SELECT median_filter(v, 3) over() FROM (VALUES (1), (1.1), (0.9), (1.1), (0.95), (2.1), (1.95), (2.0), (2.05), (3.11), (2.99), (3.05), (3.0)) as t(v);
SELECT median_filter(v, 3) over() FROM (VALUES (1), (1.1), (0.9), (1.1), (0.95), (2.1), (1.95), (2.0), (2.05), (3.11), (2.99), (3.05), (3.0)) as t(v);