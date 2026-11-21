FROM python:3.12-alpine AS sql-builder

WORKDIR /build
COPY data/sql/base /build/base
COPY deploy/mssql/scripts/build-sql-bundle.py /build/build-sql-bundle.py
RUN SRC_DB_ROOT=/build/base OUT_SQL_DIR=/build/sql python3 /build/build-sql-bundle.py

FROM mcr.microsoft.com/mssql/server:2022-latest

USER root
RUN mkdir -p /opt/dn-init/sql /opt/dn-init/scripts /opt/dn-init/updates
COPY --from=sql-builder /build/sql/ /opt/dn-init/sql/
COPY deploy/mssql/scripts/entrypoint.sh deploy/mssql/scripts/init-db.sh /opt/dn-init/scripts/
RUN sed -i 's/\r$//' /opt/dn-init/scripts/*.sh /opt/dn-init/sql/*.sql \
    && chmod 0755 /opt/dn-init/scripts/entrypoint.sh /opt/dn-init/scripts/init-db.sh \
    && chown -R mssql:mssql /opt/dn-init

USER mssql
ENTRYPOINT ["/opt/dn-init/scripts/entrypoint.sh"]
CMD ["/opt/mssql/bin/sqlservr"]
