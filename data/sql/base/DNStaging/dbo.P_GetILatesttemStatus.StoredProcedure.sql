USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetILatesttemStatus]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 16
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-07
description : ?? ??? ???? ???? ?? ??? ?????.
result set : ItemLevel/tinyint, SealCount/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, ItemPotential/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
6 = ItemPotential? ????? ?? at 2010-11-08
7~12 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
14 = ItemAttributeLogs ??? ?? ?? ???? ??  by ??? at 2012-04-06
15 = LogDate???? ?? ????? ?? by ??? at 2012-04-18
16 = ??? ???? ?? by ??? at 2012-06-15
**/

CREATE PROCEDURE [dbo].[P_GetILatesttemStatus]
	  @inbItemSerial bigint --// ??? ???
	, @dt2BasisDate datetime2(0) --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName1 nvarchar(50) = N'ItemLogs'
	, @nvcTblName2 nvarchar(50) = N'ItemAttributeLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(MM, -3, @dt2BasisDate), 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2BasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT1 nvarchar(max) = N''
	, @nvcStmtOUT2 nvarchar(max) = N'';


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
		, @nvcTblName = @nvcTblName1
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT1 OUTPUT;
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName2
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT2 OUTPUT;
		
	SET @nvcStmt = N'
	DECLARE @tblTemp table (
		LogDate datetime2(0) NOT NULL,
		ItemLevel tinyint NULL,
		SealCount tinyint NULL,
		SoulBoundFlag bit NULL,
		ItemOption tinyint NULL,
		ItemPotential tinyint NULL
	);

	INSERT @tblTemp (LogDate, ItemLevel, SealCount, SoulBoundFlag, ItemOption, ItemPotential)
	SELECT LogDate, ItemLevel, SealCount, SoulBoundFlag, ItemOption, ItemPotential
	FROM (' + @nvcStmtOUT1 + N') V
	WHERE ItemSerial = @inbItemSerial AND LogDate <= @dt2BasisDate;
	'
	IF @nvcStmtOUT2 > ''	
	BEGIN
	SET @nvcStmt +=
		N'
			INSERT @tblTemp (LogDate, ItemLevel, SealCount, SoulBoundFlag, ItemOption)
			SELECT LogDate
				, CASE WHEN ItemLevelBefore <> ItemLevelAfter THEN ItemLevelAfter ELSE NULL END
				, CASE WHEN SealCountBefore <> SealCountAfter THEN SealCountAfter ELSE NULL END
				, CASE WHEN SoulBoundFlagBefore <> SoulBoundFlagAfter THEN SoulBoundFlagAfter ELSE NULL END
				, NULL
			FROM (' + @nvcStmtOUT2 + N') V
			WHERE ItemSerial = @inbItemSerial AND LogDate <= @dt2BasisDate;
		'
	END
	SET @nvcStmt +=
		N'
		SELECT (SELECT TOP (1) ItemLevel FROM @tblTemp WHERE ItemLevel IS NOT NULL ORDER BY LogDate DESC) AS ItemLevel
		, (SELECT TOP (1) SealCount FROM @tblTemp WHERE SealCount IS NOT NULL ORDER BY LogDate DESC) AS SealCount
		, (SELECT TOP (1) SoulBoundFlag FROM @tblTemp WHERE SoulBoundFlag IS NOT NULL ORDER BY LogDate DESC) AS SoulBoundFlag
		, (SELECT TOP (1) ItemOption FROM @tblTemp WHERE ItemOption IS NOT NULL ORDER BY LogDate DESC) AS ItemOption
		, (SELECT TOP (1) ItemPotential FROM @tblTemp WHERE ItemPotential IS NOT NULL ORDER BY LogDate DESC) AS ItemPotential;
	';
	EXEC sp_executesql @nvcStmt, N'@inbItemSerial bigint, @dt2BasisDate datetime2(0)', @inbItemSerial = @inbItemSerial, @dt2BasisDate = @dt2BasisDate;
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
