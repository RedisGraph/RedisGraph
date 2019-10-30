#!/usr/bin/env bash

INSTALLPATH=${INSTALLPATH:-"/data"}

# Ensure DATA DIR available
mkdir -p ${INSTALLPATH}
chmod a+rwx ${INSTALLPATH}

BUCKET=performance-cto-group-public
BUCKETPATH=benchmarks/redisgraph/graph-database-benchmark/graph500/rdb

for datafile in graph500.rdb; do
  if [ ! -f "${INSTALLPATH}/${datafile}" ]; then
    echo "${datafile} not found locally at ${INSTALLPATH}. Retrieving..."
    echo "Getting data: $datafile"
    wget https://$BUCKET.s3.amazonaws.com/$BUCKETPATH/$datafile
    chmod 644 $datafile
    mv $datafile "${INSTALLPATH}/${datafile}"
    echo "Data file at data: ${INSTALLPATH}/${datafile}"
  else
    echo "${datafile} found locally at ${INSTALLPATH}/${datafile}. No need to retrieve again."
  fi

done

