USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_NestClearResult]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-10-05

description :
	(TIMOSE) ??? ??? ??? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	4 = ????
	5 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_TMS_NestClearResult]
	@dtBasisDate date
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'StageEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtBasisDate, 112)
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';
	
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
		
	--SELECT N'      , @intJobCode_' + CONVERT(nvarchar(max), JobCode) + N' int -- ' + CONVERT(nvarchar(max), JobName) FROM dbo.DimJobs WITH (NOLOCK) ORDER BY JobCode;
	SET @nvcStmt = N'
	DECLARE @intSeqNo int
		  , @intMaxSeqNo int
		  , @intMapID int
		  , @intJobCode_1 int -- ???
		  , @intJobCode_2 int -- ??
		  , @intJobCode_3 int -- ????
		  , @intJobCode_4 int -- ???
		  , @intJobCode_5 int -- ????
		  --, @intJobCode_10 int -- ???
		  , @intJobCode_11 int -- ?????
		  , @intJobCode_12 int -- ????
		  --, @intJobCode_13 int -- ???
		  , @intJobCode_14 int -- ?????
		  , @intJobCode_15 int -- ????
		  --, @intJobCode_16 int -- ??
		  , @intJobCode_17 int -- ??????
		  , @intJobCode_18 int -- ????
		  --, @intJobCode_19 int -- ??
		  , @intJobCode_20 int -- ???
		  --, @intJobCode_21 int -- ??
		  , @intJobCode_22 int -- ????
		  , @intJobCode_23 int -- ??????
		  , @intJobCode_24 int -- ???
		  , @intJobCode_25 int -- ????
		  , @intJobCode_26 int -- ??????
		  , @intJobCode_29 int -- ????
		  , @intJobCode_30 int -- ????
		  , @intJobCode_31 int -- ????
		  , @intJobCode_32 int -- ????
		  , @intJobCode_35 int -- ????
		  , @intJobCode_36 int -- ?????
		  , @intJobCode_37 int -- ???
		  , @intJobCode_38 int -- ????
		  , @intJobCode_41 int -- ???
		  , @intJobCode_42 int -- ?????
		  , @intJobCode_43 int -- ???
		  , @intJobCode_44 int -- ????
		  --, @intJobCode_45 int -- ?????
		  , @intJobCode_46 int -- ????
		  , @intJobCode_47 int -- ????
		  , @intJobCode_48 int -- ?????
		  , @intRowCount int

	CREATE TABLE #Temp_MapID (SeqNo int identity, MapID int);

	--SELECT N''     , ('' + CONVERT(nvarchar(max), MapID) + '')'' FROM dbo.DimMaps WITH (NOLOCK) WHERE MapName LIKE N''%???%'' ORDER BY MapID;

	INSERT #Temp_MapID (MapID)
	VALUES (14002)
		 , (14003)
		 , (14011)
		 , (14021)
		 , (14031)
		 , (14032)
		 , (14033)
		 , (14034)
		 , (14041)
		 , (14042)
		 , (14051)
		 , (14052)
		 , (14071)
		 , (14072)
		 , (14073)
		 , (14077)
		 , (14081)
		 , (14082)
		 , (14083)
		 , (14085)
		 , (14090)
		 , (14091)
		 , (14092);

	SELECT @intSeqNo = 1, @intMaxSeqNo = MAX(SeqNo) FROM #Temp_MapID;

	WHILE (@intSeqNo <= @intMaxSeqNo)
	BEGIN

		SELECT @intMapID = MapID FROM #Temp_MapID WHERE SeqNo = @intSeqNo;

		SET @intJobCode_1 = 0;
		SET @intJobCode_2 = 0;
		SET @intJobCode_3 = 0;
		SET @intJobCode_4 = 0;
		SET @intJobCode_5 = 0;
		SET @intJobCode_11 = 0;
		SET @intJobCode_12 = 0;
		SET @intJobCode_14 = 0;
		SET @intJobCode_15 = 0;
		SET @intJobCode_17 = 0;
		SET @intJobCode_18 = 0;
		SET @intJobCode_20 = 0;
		SET @intJobCode_22 = 0;
		SET @intJobCode_23 = 0;
		SET @intJobCode_24 = 0;
		SET @intJobCode_25 = 0;
		SET @intJobCode_26 = 0;
		SET @intJobCode_29 = 0;
		SET @intJobCode_30 = 0;
		SET @intJobCode_31 = 0;
		SET @intJobCode_32 = 0;
		SET @intJobCode_35 = 0;
		SET @intJobCode_36 = 0;
		SET @intJobCode_37 = 0;
		SET @intJobCode_38 = 0;
		SET @intJobCode_41 = 0;
		SET @intJobCode_42 = 0;
		SET @intJobCode_43 = 0;
		SET @intJobCode_44 = 0;
		SET @intJobCode_46 = 0;
		SET @intJobCode_47 = 0;
		SET @intJobCode_48 = 0;
		SELECT @intJobCode_1 = @intJobCode_1 + CASE WHEN SEL.JobCode = 1 THEN 1 ELSE 0 END
			 , @intJobCode_2 = @intJobCode_2 + CASE WHEN SEL.JobCode = 2 THEN 1 ELSE 0 END
			 , @intJobCode_3 = @intJobCode_3 + CASE WHEN SEL.JobCode = 3 THEN 1 ELSE 0 END
			 , @intJobCode_4 = @intJobCode_4 + CASE WHEN SEL.JobCode = 4 THEN 1 ELSE 0 END
			 , @intJobCode_5 = @intJobCode_5 + CASE WHEN SEL.JobCode = 5 THEN 1 ELSE 0 END
			 , @intJobCode_11 = @intJobCode_11 + CASE WHEN SEL.JobCode = 11 THEN 1 ELSE 0 END
			 , @intJobCode_12 = @intJobCode_12 + CASE WHEN SEL.JobCode = 12 THEN 1 ELSE 0 END
			 , @intJobCode_14 = @intJobCode_14 + CASE WHEN SEL.JobCode = 14 THEN 1 ELSE 0 END
			 , @intJobCode_15 = @intJobCode_15 + CASE WHEN SEL.JobCode = 15 THEN 1 ELSE 0 END
			 , @intJobCode_17 = @intJobCode_17 + CASE WHEN SEL.JobCode = 17 THEN 1 ELSE 0 END
			 , @intJobCode_18 = @intJobCode_18 + CASE WHEN SEL.JobCode = 18 THEN 1 ELSE 0 END
			 , @intJobCode_20 = @intJobCode_20 + CASE WHEN SEL.JobCode = 20 THEN 1 ELSE 0 END
			 , @intJobCode_22 = @intJobCode_22 + CASE WHEN SEL.JobCode = 22 THEN 1 ELSE 0 END
			 , @intJobCode_23 = @intJobCode_23 + CASE WHEN SEL.JobCode = 23 THEN 1 ELSE 0 END
			 , @intJobCode_24 = @intJobCode_24 + CASE WHEN SEL.JobCode = 24 THEN 1 ELSE 0 END
			 , @intJobCode_25 = @intJobCode_25 + CASE WHEN SEL.JobCode = 25 THEN 1 ELSE 0 END
			 , @intJobCode_26 = @intJobCode_26 + CASE WHEN SEL.JobCode = 26 THEN 1 ELSE 0 END
			 , @intJobCode_29 = @intJobCode_29 + CASE WHEN SEL.JobCode = 29 THEN 1 ELSE 0 END
			 , @intJobCode_30 = @intJobCode_30 + CASE WHEN SEL.JobCode = 30 THEN 1 ELSE 0 END
			 , @intJobCode_31 = @intJobCode_31 + CASE WHEN SEL.JobCode = 31 THEN 1 ELSE 0 END
			 , @intJobCode_32 = @intJobCode_32 + CASE WHEN SEL.JobCode = 32 THEN 1 ELSE 0 END
			 , @intJobCode_35 = @intJobCode_35 + CASE WHEN SEL.JobCode = 35 THEN 1 ELSE 0 END
			 , @intJobCode_36 = @intJobCode_36 + CASE WHEN SEL.JobCode = 36 THEN 1 ELSE 0 END
			 , @intJobCode_37 = @intJobCode_37 + CASE WHEN SEL.JobCode = 37 THEN 1 ELSE 0 END
			 , @intJobCode_38 = @intJobCode_38 + CASE WHEN SEL.JobCode = 38 THEN 1 ELSE 0 END
			 , @intJobCode_41 = @intJobCode_41 + CASE WHEN SEL.JobCode = 41 THEN 1 ELSE 0 END
			 , @intJobCode_42 = @intJobCode_42 + CASE WHEN SEL.JobCode = 42 THEN 1 ELSE 0 END
			 , @intJobCode_43 = @intJobCode_43 + CASE WHEN SEL.JobCode = 43 THEN 1 ELSE 0 END
			 , @intJobCode_44 = @intJobCode_44 + CASE WHEN SEL.JobCode = 44 THEN 1 ELSE 0 END
			 , @intJobCode_46 = @intJobCode_46 + CASE WHEN SEL.JobCode = 46 THEN 1 ELSE 0 END
			 , @intJobCode_47 = @intJobCode_47 + CASE WHEN SEL.JobCode = 47 THEN 1 ELSE 0 END
			 , @intJobCode_48 = @intJobCode_48 + CASE WHEN SEL.JobCode = 48 THEN 1 ELSE 0 END
		  FROM (' + @nvcStmtOUT + N') SEL
		 WHERE SEL.StageStartDate >= CONVERT(smalldatetime, @dtBasisDate)
		   AND SEL.StageStartDate < CONVERT(smalldatetime, DATEADD(day, 1, @dtBasisDate))
		   AND SEL.ClearFlag = 1
		   AND SEL.MapID = @intMapID;

		SELECT @intRowCount = @@ROWCOUNT;

		IF OBJECT_ID(''tempdb..#Temp_NestClearResult'') IS NULL AND @intRowCount > 0
		BEGIN
			SELECT @dtBasisDate      AS BasisDate
				 , @intMapID         AS MapID
				 , @intJobCode_1     AS JobCode_1
				 , @intJobCode_2     AS JobCode_2
				 , @intJobCode_3     AS JobCode_3
				 , @intJobCode_4     AS JobCode_4
				 , @intJobCode_5     AS JobCode_5
				 , @intJobCode_11    AS JobCode_11
				 , @intJobCode_12    AS JobCode_12
				 , @intJobCode_14    AS JobCode_14
				 , @intJobCode_15    AS JobCode_15
				 , @intJobCode_17    AS JobCode_17
				 , @intJobCode_18    AS JobCode_18
				 , @intJobCode_20    AS JobCode_20
				 , @intJobCode_22    AS JobCode_22
				 , @intJobCode_23    AS JobCode_23
				 , @intJobCode_24    AS JobCode_24
				 , @intJobCode_25    AS JobCode_25
				 , @intJobCode_26    AS JobCode_26
				 , @intJobCode_29    AS JobCode_29
				 , @intJobCode_30    AS JobCode_30
				 , @intJobCode_31    AS JobCode_31
				 , @intJobCode_32    AS JobCode_32
				 , @intJobCode_35    AS JobCode_35
				 , @intJobCode_36    AS JobCode_36
				 , @intJobCode_37    AS JobCode_37
				 , @intJobCode_38    AS JobCode_38
				 , @intJobCode_41    AS JobCode_41
				 , @intJobCode_42    AS JobCode_42
				 , @intJobCode_43    AS JobCode_43
				 , @intJobCode_44    AS JobCode_44
				 , @intJobCode_46    AS JobCode_46
				 , @intJobCode_47    AS JobCode_47
				 , @intJobCode_48    AS JobCode_48
			  INTO #Temp_NestClearResult;
		END
		ELSE IF OBJECT_ID(''tempdb..#Temp_NestClearResult'') IS NOT NULL AND @intRowCount > 0
		BEGIN
			INSERT #Temp_NestClearResult
			SELECT @dtBasisDate
				 , @intMapID
				 , @intJobCode_1
				 , @intJobCode_2
				 , @intJobCode_3
				 , @intJobCode_4
				 , @intJobCode_5
				 , @intJobCode_11
				 , @intJobCode_12
				 , @intJobCode_14
				 , @intJobCode_15
				 , @intJobCode_17
				 , @intJobCode_18
				 , @intJobCode_20
				 , @intJobCode_22
				 , @intJobCode_23
				 , @intJobCode_24
				 , @intJobCode_25
				 , @intJobCode_26
				 , @intJobCode_29
				 , @intJobCode_30
				 , @intJobCode_31
				 , @intJobCode_32
				 , @intJobCode_35
				 , @intJobCode_36
				 , @intJobCode_37
				 , @intJobCode_38
				 , @intJobCode_41
				 , @intJobCode_42
				 , @intJobCode_43
				 , @intJobCode_44
				 , @intJobCode_46
				 , @intJobCode_47
				 , @intJobCode_48;
		END

		SELECT @intSeqNo = @intSeqNo + 1;

	END

	IF OBJECT_ID(''tempdb..#Temp_NestClearResult'') IS NOT NULL
	BEGIN
		SELECT NULL AS PublisherID
			 , NULL AS NationalityCode
			 , NULL AS PartitionID
			 , NULL AS WorldID
			 , BasisDate
			 , MapID
			 , JobCode_1
			 , JobCode_2
			 , JobCode_3
			 , JobCode_4
			 , JobCode_5
			 , JobCode_11
			 , JobCode_12
			 , JobCode_14
			 , JobCode_15
			 , JobCode_17
			 , JobCode_18
			 , JobCode_20
			 , JobCode_22
			 , JobCode_23
			 , JobCode_24
			 , JobCode_25
			 , JobCode_26
			 , JobCode_29
			 , JobCode_30
			 , JobCode_31
			 , JobCode_32
			 , JobCode_35
			 , JobCode_36
			 , JobCode_37
			 , JobCode_38
			 , JobCode_41
			 , JobCode_42
			 , JobCode_43
			 , JobCode_44
			 , JobCode_46
			 , JobCode_47
			 , JobCode_48
		  FROM #Temp_NestClearResult;

		DROP TABLE #Temp_NestClearResult;
	END
	ELSE
	BEGIN
		SELECT NULL AS PublisherID
			 , NULL AS NationalityCode
			 , NULL AS PartitionID
			 , NULL AS WorldID
			 , @dtBasisDate AS BasisDate
			 , NULL AS MapID
			 , 0 AS JobCode_1
			 , 0 AS JobCode_2
			 , 0 AS JobCode_3
			 , 0 AS JobCode_4
			 , 0 AS JobCode_5
			 , 0 AS JobCode_11
			 , 0 AS JobCode_12
			 , 0 AS JobCode_14
			 , 0 AS JobCode_15
			 , 0 AS JobCode_17
			 , 0 AS JobCode_18
			 , 0 AS JobCode_20
			 , 0 AS JobCode_22
			 , 0 AS JobCode_23
			 , 0 AS JobCode_24
			 , 0 AS JobCode_25
			 , 0 AS JobCode_26
			 , 0 AS JobCode_29
			 , 0 AS JobCode_30
			 , 0 AS JobCode_31
			 , 0 AS JobCode_32
			 , 0 AS JobCode_35
			 , 0 AS JobCode_36
			 , 0 AS JobCode_37
			 , 0 AS JobCode_38
			 , 0 AS JobCode_41
			 , 0 AS JobCode_42
			 , 0 AS JobCode_43
			 , 0 AS JobCode_44
			 , 0 AS JobCode_46
			 , 0 AS JobCode_47
			 , 0 AS JobCode_48
	END

	DROP TABLE #Temp_MapID;
	';
	EXEC sp_executesql @nvcStmt, N'@dtBasisDate date', @dtBasisDate = @dtBasisDate;
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
