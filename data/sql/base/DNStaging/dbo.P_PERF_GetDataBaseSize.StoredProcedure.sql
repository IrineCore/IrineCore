USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_PERF_GetDataBaseSize]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-08-02
description : ?? ?????? ??? ?????.(???? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_PERF_GetDataBaseSize]
	  @dtmBasisDate datetime --// ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intWorldID int

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END
BEGIN TRY
	SELECT @dtmBasisDate AS BasisDate, @@SERVERNAME AS ServerName
         , A.DatabaseName
		 , A.DatabaseSize
		 , A.DataFileSize
		 , A.LogFileSize
		 , B.ReservedSize
		 , A.DataFileSize - B.ReservedSize AS UnAllocatedSize
		 , B.UsedSize
		 , B.IndexUsedSize
		 , B.UnUsedSize
	FROM
	   (SELECT DB_NAME() AS DatabaseName
			 , SUM(CONVERT(BIGINT,CASE TYPE WHEN 0 THEN SIZE ELSE 0 END))* 8 + SUM(CONVERT(BIGINT,CASE TYPE WHEN 1 THEN SIZE ELSE 0 END))* 8 AS DatabaseSize
			 , SUM(CONVERT(BIGINT,CASE TYPE WHEN 0 THEN SIZE ELSE 0 END))* 8 as DataFileSize
			 , SUM(CONVERT(BIGINT,CASE TYPE WHEN 1 THEN SIZE ELSE 0 END))* 8 as LogFileSize
		  FROM sys.database_files
		) A  
	INNER JOIN
		  (SELECT DB_NAME() AS DatabaseName
			    , (ReservedPages * 8) AS ReservedSize
			    , (Pages * 8) AS UsedSize
			    , ((UsedPages - Pages) * 8) AS IndexUsedSize
			    , ((ReservedPages - UsedPages) * 8) AS UnUsedSize
		  FROM
			   (SELECT SUM(A.TOTAL_PAGES) AS ReservedPages
					 , SUM(A.USED_PAGES) AS UsedPages
					 , SUM(CASE WHEN IT.INTERNAL_TYPE IN (202,204) THEN 0
										WHEN A.TYPE <> 1 THEN A.USED_PAGES
										WHEN P.INDEX_ID < 2 THEN A.DATA_PAGES
										ELSE 0 END) AS Pages
				  FROM sys.partitions P 
					INNER JOIN sys.allocation_units A ON P.PARTITION_ID = A.CONTAINER_ID
					LEFT JOIN sys.internal_tables IT ON P.OBJECT_ID = IT.OBJECT_ID
			   ) X
	   )B
	ON A.DatabaseName = B.DatabaseName
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
