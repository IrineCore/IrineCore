USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyMapStay]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-10
description : ?? ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~2 = ???? by ??? at 2013-04-10
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyMapStay]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'MapStayLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @datBasisDate = CAST(ISNULL(@datBasisDate, GETDATE()) AS date);

	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
	
	SET @nvcStmt = N'
	/**_# [ST_DailyMapStay] ???? INSERT???.*/
	INSERT dbo.ST_DailyMapStay (BasisDate, WorldID, JobCode, CharacterLevel, MapID, PlayTime_SUM_SEC, EntranceCount)
	SELECT CAST(V.ExitDate AS date) AS BasisDate, dc.WorldID, V.JobCode, V.CharacterLevel, V.MapID
		, SUM(DATEDIFF(SS, EntranceDate, ExitDate)) AS PlayTime_SUM_SEC
		, COUNT(*) AS EntranceCount
	FROM (' + @nvcStmtOUT + N') V
	INNER JOIN dbo.DimCharacters dc WITH (NOLOCK) ON dc.CharacterID = V.CharacterID
	WHERE V.ExitDate >= @datBasisDate AND V.ExitDate < DATEADD(DD, 1, @datBasisDate)
	GROUP BY CAST(V.ExitDate AS date), dc.WorldID, V.JobCode, V.CharacterLevel, V.MapID
	OPTION (MAXDOP 1);
	';
	EXEC sp_executesql @nvcStmt, N'@datBasisDate date', @datBasisDate = @datBasisDate;
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
