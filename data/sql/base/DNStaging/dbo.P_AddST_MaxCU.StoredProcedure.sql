USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_MaxCU]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-23
description : ?? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
History
20101021 = ????? ??? ??? ?? ??? ??
4~6 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
7 = ??, ?? ?? ?? by ??? at 2013-04-08
**/

CREATE PROCEDURE [dbo].[P_AddST_MaxCU]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CUCountLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';
	
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dtmBasisDate = CAST(CAST(@dtmBasisDate AS date) AS datetime);

	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	

	SET @nvcStmt = N'
/**_# ST_HourlyMaxCU ???? ??? ??*/
	INSERT dbo.ST_HourlyMaxCU (BasisDate, WorldID, CUCount, LogDate, CUCountMIN, CUCountAVG, ChannelPartnerCode)
	SELECT
		  DATEADD(HOUR, DATEDIFF(HOUR, 0, SUBSTRING(MAX(Data), 11, 20)), 0) BasisDate, WorldID, LEFT(MAX(Data), 10) * 1 AS CUCount, RIGHT(MAX(Data), 20) AS LogDate, LEFT(MIN(Data), 10) * 1 AS CUCountMIN, AVG(LEFT(Data, 10) * 1) AS CUCountAVG, NULL
	FROM
	(
		SELECT 
			  DATEPART(hh, LogDate) AS LogHour, WorldID, STR(MaxCUCount) + CONVERT(CHAR(20), LogDate, 120) AS Data
		FROM 
		(
			SELECT LogDate, WorldID, SUM(CUCount) AS MaxCUCount 
			FROM (' + @nvcStmtOUT + N') V
			GROUP BY LogDate, WorldID
		) MaxCUa
	) MaxCUb
	GROUP BY WorldID, LogHour
	UNION ALL
	SELECT
		  DATEADD(HOUR, DATEDIFF(HOUR, 0, SUBSTRING(MAX(Data), 11, 20)), 0) BasisDate, WorldID, LEFT(MAX(Data), 10) * 1 AS CUCount, RIGHT(MAX(Data), 20) AS LogDate, LEFT(MIN(Data), 10) * 1 AS CUCountMIN, AVG(LEFT(Data, 10) * 1) AS CUCountAVG, ChannelPartnerCode
	FROM
	(
		SELECT 
			  DATEPART(hh, LogDate) AS LogHour, WorldID, ChannelPartnerCode, STR(CUCount) + CONVERT(CHAR(20), LogDate, 120) AS Data
		FROM (' + @nvcStmtOUT + N') V
		WHERE LogDate >= @dtmBasisDate AND LogDate < DATEADD(DAY, 1, @dtmBasisDate)
	) MaxCU
	GROUP BY WorldID, LogHour, ChannelPartnerCode
	UNION ALL
	SELECT
		  DATEADD(HOUR, DATEDIFF(HOUR, 0, SUBSTRING(MAX(Data), 11, 20)), 0), WorldID = NULL, LEFT(MAX(Data), 10) * 1 AS CUCount, RIGHT(MAX(Data), 20) AS LogDate, LEFT(MIN(Data), 10) * 1 AS CUCountMIN, AVG(LEFT(Data, 10) * 1) AS CUCountAVG, NULL
	FROM
	(
		SELECT 
			  DATEPART(hh, LogDate) AS LogHour, STR(MaxCUCount) + CONVERT(CHAR(20), LogDate, 120) AS Data
		FROM 
		(
			SELECT LogDate, SUM(CUCount) AS MaxCUCount 
			FROM (' + @nvcStmtOUT + N') V
			WHERE LogDate >= @dtmBasisDate AND LogDate < DATEADD(DAY, 1, @dtmBasisDate)
			GROUP BY LogDate
		) MaxCUa
	) MaxCUb
	GROUP BY LogHour;
	';
	EXEC sp_executesql @nvcStmt, N'@dtmBasisDate datetime', @dtmBasisDate = @dtmBasisDate;
	
/**_# ST_DailyMaxCU ???? ??? ??*/
	INSERT dbo.ST_DailyMaxCU (BasisDate, WorldID, CUCount, LogDate, ChannelPartnerCode)
	SELECT @dtmBasisDate, WorldID, LEFT(MAX(Data), 10) * 1 AS CUCount, RIGHT(MAX(Data), 20) AS LogDate, ChannelPartnerCode
	FROM
	(
		SELECT WorldID, STR(CUCount) + CONVERT(CHAR(20), LogDate, 120) AS Data, ChannelPartnerCode
		FROM dbo.ST_HourlyMaxCU
		WHERE BasisDate >= @dtmBasisDate AND BasisDate < DATEADD(DAY, 1, @dtmBasisDate)
	) MaxCU
	GROUP BY WorldID, ChannelPartnerCode;
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
