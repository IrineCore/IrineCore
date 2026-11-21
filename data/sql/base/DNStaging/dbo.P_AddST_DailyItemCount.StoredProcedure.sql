USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyItemCount]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-09
description : ?? ??? ??/???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~2 = ???? by ??? at 2013-04-09
3~4 = ItemLocationCode ?? by ??? at 2013-04-26
5~7 = JobCode ??, CharacterLevel & ItemLevel ??? by ??? at 2013-04-29
10 = UNION ALL ?? by ??? at 2013-05-20
11 = ?? ?? ?? by ??? at 2013-08-23
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyItemCount]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'ItemLogs'
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
	/**_# [ST_DailyItemCount] ???? INSERT???.*/
	INSERT dbo.ST_DailyItemCount (BasisDate, WorldID, CharacterLevel, JobCode, ItemID, ItemLevel, ItemUseCode, ItemOptionGroupCode, ActiveUserFlag, ItemLocationCode, ItemCount, ItemCategoryID)
	SELECT CAST(V.LogDate AS date) AS BasisDate, V.WorldID
		, CASE WHEN dc.CharacterLevel >= 1 AND dc.CharacterLevel <= 5 THEN 1 WHEN dc.CharacterLevel >= 6 AND dc.CharacterLevel <= 10 THEN 2
			WHEN dc.CharacterLevel >= 11 AND dc.CharacterLevel <= 15 THEN 3 WHEN dc.CharacterLevel >= 16 AND dc.CharacterLevel <= 24 THEN 4
			WHEN dc.CharacterLevel >= 25 AND dc.CharacterLevel <= 32 THEN 5 WHEN dc.CharacterLevel >= 33 AND dc.CharacterLevel <= 39 THEN 6
			WHEN dc.CharacterLevel = 40 THEN 7 WHEN dc.CharacterLevel >= 41 AND dc.CharacterLevel <= 49 THEN 8
			WHEN dc.CharacterLevel = 50 THEN 9 WHEN dc.CharacterLevel >= 51 AND dc.CharacterLevel <= 59 THEN 10
			WHEN dc.CharacterLevel = 60 THEN 11 WHEN dc.CharacterLevel >= 61 AND dc.CharacterLevel <= 69 THEN 12
			WHEN dc.CharacterLevel = 70 THEN 13 WHEN dc.CharacterLevel >= 71 AND dc.CharacterLevel <= 79 THEN 14
			WHEN dc.CharacterLevel = 80 THEN 15 WHEN dc.CharacterLevel >= 81 AND dc.CharacterLevel <= 89 THEN 16
			WHEN dc.CharacterLevel = 90 THEN 17 WHEN dc.CharacterLevel >= 91 AND dc.CharacterLevel <= 99 THEN 18
			WHEN dc.CharacterLevel = 100 THEN 19 END AS CharacterLevel, NULL AS JobCode, NULL AS ItemID
		, CASE WHEN V.ItemLevel > 0 AND V.ItemLevel < 7 THEN 6 ELSE V.ItemLevel END AS ItemLevel
		, CASE WHEN V.ItemLogCode IN (1,2,3,4,5,6,7,8,9,11,13,14,15,16,17,28,29,31,37,39,43,47,48,49,50,51,52,53,54,55,57,59,64,67,73,74,78) THEN 1
			WHEN V.ItemLogCode IN (20,21,22,23,24,25,26,27,30,38,42,56,58,77) THEN 2
			WHEN V.ItemLogCode = 12 AND V.ItemLogFKey1 > 1000 THEN 4
			WHEN V.ItemLogCode = 12 AND (V.ItemLogFKey1 = 0 OR V.ItemLogFKey1 IS NULL) THEN 5 END AS ItemUseCode
		, CASE WHEN V.ItemOption >= il.ItemOption1Start AND V.ItemOption <= il.ItemOption1End THEN 1
			WHEN V.ItemOption >= il.ItemOption2Start AND V.ItemOption <= il.ItemOption2End THEN 2
			WHEN V.ItemOption >= il.ItemOption3Start AND V.ItemOption <= il.ItemOption3End THEN 3
			ELSE 0 END AS ItemOptionGroupCode
		, CASE WHEN dc.LastLoginDate >= DATEADD(DD, -31, getdate()) THEN 1 ELSE 0 END AS ActiveUserFlag
		, NULL AS ItemLocationCode
		, SUM(CASE WHEN V.ItemCount = V.ItemCountBefore THEN V.ItemCount ELSE ABS(V.ItemCount - ItemCountBefore) END) AS ItemCount
		, il.ItemCategoryID
	FROM (' + @nvcStmtOUT + N') V 
	INNER JOIN dbo.ItemListForCount il WITH (NOLOCK) ON il.ItemID = V.ItemID
	INNER JOIN dbo.DimCharacters dc WITH (NOLOCK) ON dc.CharacterID = V.OwnerCharacterID
	WHERE V.LogDate >= @datBasisDate AND V.LogDate < DATEADD(DD, 1, @datBasisDate)
	AND V.ItemLogCode IN (1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,28,29,31,37,39,43,47,48,49,50,51,52,53,54,55,57,59,64,67,73,74,78, 20,21,22,23,24,25,26,27,30,38,42,56,58,77)
	GROUP BY CAST(V.LogDate AS date), V.WorldID
		, CASE WHEN dc.CharacterLevel >= 1 AND dc.CharacterLevel <= 5 THEN 1 WHEN dc.CharacterLevel >= 6 AND dc.CharacterLevel <= 10 THEN 2
			WHEN dc.CharacterLevel >= 11 AND dc.CharacterLevel <= 15 THEN 3 WHEN dc.CharacterLevel >= 16 AND dc.CharacterLevel <= 24 THEN 4
			WHEN dc.CharacterLevel >= 25 AND dc.CharacterLevel <= 32 THEN 5 WHEN dc.CharacterLevel >= 33 AND dc.CharacterLevel <= 39 THEN 6
			WHEN dc.CharacterLevel = 40 THEN 7 WHEN dc.CharacterLevel >= 41 AND dc.CharacterLevel <= 49 THEN 8
			WHEN dc.CharacterLevel = 50 THEN 9 WHEN dc.CharacterLevel >= 51 AND dc.CharacterLevel <= 59 THEN 10
			WHEN dc.CharacterLevel = 60 THEN 11 WHEN dc.CharacterLevel >= 61 AND dc.CharacterLevel <= 69 THEN 12
			WHEN dc.CharacterLevel = 70 THEN 13 WHEN dc.CharacterLevel >= 71 AND dc.CharacterLevel <= 79 THEN 14
			WHEN dc.CharacterLevel = 80 THEN 15 WHEN dc.CharacterLevel >= 81 AND dc.CharacterLevel <= 89 THEN 16
			WHEN dc.CharacterLevel = 90 THEN 17 WHEN dc.CharacterLevel >= 91 AND dc.CharacterLevel <= 99 THEN 18
			WHEN dc.CharacterLevel = 100 THEN 19 END
		, CASE WHEN V.ItemLevel > 0 AND V.ItemLevel < 7 THEN 6 ELSE V.ItemLevel END
		, CASE WHEN V.ItemLogCode IN (1,2,3,4,5,6,7,8,9,11,13,14,15,16,17,28,29,31,37,39,43,47,48,49,50,51,52,53,54,55,57,59,64,67,73,74,78) THEN 1
			WHEN V.ItemLogCode IN (20,21,22,23,24,25,26,27,30,38,42,56,58,77) THEN 2
			WHEN V.ItemLogCode = 12 AND V.ItemLogFKey1 > 1000 THEN 4
			WHEN V.ItemLogCode = 12 AND (V.ItemLogFKey1 = 0 OR V.ItemLogFKey1 IS NULL) THEN 5 END
		, CASE WHEN V.ItemOption >= il.ItemOption1Start AND V.ItemOption <= il.ItemOption1End THEN 1
			WHEN V.ItemOption >= il.ItemOption2Start AND V.ItemOption <= il.ItemOption2End THEN 2
			WHEN V.ItemOption >= il.ItemOption3Start AND V.ItemOption <= il.ItemOption3End THEN 3
			ELSE 0 END
		, CASE WHEN dc.LastLoginDate >= DATEADD(DD, -31, getdate()) THEN 1 ELSE 0 END, il.ItemCategoryID
	OPTION (MAXDOP 1);
	
	INSERT dbo.ST_DailyItemCount (BasisDate, WorldID, CharacterLevel, JobCode, ItemID, ItemLevel, ItemUseCode, ItemOptionGroupCode, ActiveUserFlag, ItemLocationCode, ItemCount, ItemCategoryID)
	SELECT CAST(V.LogDate AS date) AS BasisDate, V.WorldID
		, CASE WHEN dc.CharacterLevel >= 1 AND dc.CharacterLevel <= 5 THEN 1 WHEN dc.CharacterLevel >= 6 AND dc.CharacterLevel <= 10 THEN 2
			WHEN dc.CharacterLevel >= 11 AND dc.CharacterLevel <= 15 THEN 3 WHEN dc.CharacterLevel >= 16 AND dc.CharacterLevel <= 24 THEN 4
			WHEN dc.CharacterLevel >= 25 AND dc.CharacterLevel <= 32 THEN 5 WHEN dc.CharacterLevel >= 33 AND dc.CharacterLevel <= 39 THEN 6
			WHEN dc.CharacterLevel = 40 THEN 7 WHEN dc.CharacterLevel >= 41 AND dc.CharacterLevel <= 49 THEN 8
			WHEN dc.CharacterLevel = 50 THEN 9 WHEN dc.CharacterLevel >= 51 AND dc.CharacterLevel <= 59 THEN 10
			WHEN dc.CharacterLevel = 60 THEN 11 WHEN dc.CharacterLevel >= 61 AND dc.CharacterLevel <= 69 THEN 12
			WHEN dc.CharacterLevel = 70 THEN 13 WHEN dc.CharacterLevel >= 71 AND dc.CharacterLevel <= 79 THEN 14
			WHEN dc.CharacterLevel = 80 THEN 15 WHEN dc.CharacterLevel >= 81 AND dc.CharacterLevel <= 89 THEN 16
			WHEN dc.CharacterLevel = 90 THEN 17 WHEN dc.CharacterLevel >= 91 AND dc.CharacterLevel <= 99 THEN 18
			WHEN dc.CharacterLevel = 100 THEN 19 END AS CharacterLevel, NULL AS JobCode, NULL AS ItemID
		, CASE WHEN V.ItemLevel > 0 AND V.ItemLevel < 7 THEN 6 ELSE V.ItemLevel END AS ItemLevel  
		, CASE WHEN V.ItemLogCode IN (1,2,3,4,5,6,7,8,9,11,13,14,15,16,17,28,29,31,37,39,43,47,48,49,50,51,52,53,54,55,57,59,64,67,73,74,78) THEN 1
			WHEN V.ItemLogCode IN (20,21,22,23,24,25,26,27,30,38,42,56,58,77) THEN 2
			WHEN V.ItemLogCode = 12 AND V.ItemLogFKey1 > 1000 THEN 4
			WHEN V.ItemLogCode = 12 AND (V.ItemLogFKey1 = 0 OR V.ItemLogFKey1 IS NULL) THEN 5 END AS ItemUseCode
		, CASE WHEN V.ItemOption >= il.ItemOption1Start AND V.ItemOption <= il.ItemOption1End THEN 1
			WHEN V.ItemOption >= il.ItemOption2Start AND V.ItemOption <= il.ItemOption2End THEN 2
			WHEN V.ItemOption >= il.ItemOption3Start AND V.ItemOption <= il.ItemOption3End THEN 3
			ELSE 0 END AS ItemOptionGroupCode
		, CASE WHEN dc.LastLoginDate >= DATEADD(DD, -31, getdate()) THEN 1 ELSE 0 END AS ActiveUserFlag
		, NULL AS ItemLocationCode
		, SUM(CASE WHEN V.ItemCount = V.ItemCountBefore THEN V.ItemCount ELSE ABS(V.ItemCount - ItemCountBefore) END) AS ItemCount, il.ItemCategoryID
	FROM (' + @nvcStmtOUT + N') V
	INNER JOIN dbo.ItemListForCount il WITH (NOLOCK) ON il.ItemID = V.ItemID
	INNER JOIN dbo.DimCharacters dc WITH (NOLOCK) ON dc.CharacterID = V.OwnerCharacterIDBefore
	WHERE V.LogDate >= @datBasisDate AND V.LogDate < DATEADD(DD, 1, @datBasisDate)
	AND V.ItemLogCode IN (1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,28,29,31,37,39,43,47,48,49,50,51,52,53,54,55,57,59,64,67,73,74,78, 20,21,22,23,24,25,26,27,30,38,42,56,58,77)
	GROUP BY CAST(V.LogDate AS date), V.WorldID
		, CASE WHEN dc.CharacterLevel >= 1 AND dc.CharacterLevel <= 5 THEN 1 WHEN dc.CharacterLevel >= 6 AND dc.CharacterLevel <= 10 THEN 2
			WHEN dc.CharacterLevel >= 11 AND dc.CharacterLevel <= 15 THEN 3 WHEN dc.CharacterLevel >= 16 AND dc.CharacterLevel <= 24 THEN 4
			WHEN dc.CharacterLevel >= 25 AND dc.CharacterLevel <= 32 THEN 5 WHEN dc.CharacterLevel >= 33 AND dc.CharacterLevel <= 39 THEN 6
			WHEN dc.CharacterLevel = 40 THEN 7 WHEN dc.CharacterLevel >= 41 AND dc.CharacterLevel <= 49 THEN 8
			WHEN dc.CharacterLevel = 50 THEN 9 WHEN dc.CharacterLevel >= 51 AND dc.CharacterLevel <= 59 THEN 10
			WHEN dc.CharacterLevel = 60 THEN 11 WHEN dc.CharacterLevel >= 61 AND dc.CharacterLevel <= 69 THEN 12
			WHEN dc.CharacterLevel = 70 THEN 13 WHEN dc.CharacterLevel >= 71 AND dc.CharacterLevel <= 79 THEN 14
			WHEN dc.CharacterLevel = 80 THEN 15 WHEN dc.CharacterLevel >= 81 AND dc.CharacterLevel <= 89 THEN 16
			WHEN dc.CharacterLevel = 90 THEN 17 WHEN dc.CharacterLevel >= 91 AND dc.CharacterLevel <= 99 THEN 18
			WHEN dc.CharacterLevel = 100 THEN 19 END
		, CASE WHEN V.ItemLevel > 0 AND V.ItemLevel < 7 THEN 6 ELSE V.ItemLevel END
		, CASE WHEN V.ItemLogCode IN (1,2,3,4,5,6,7,8,9,11,13,14,15,16,17,28,29,31,37,39,43,47,48,49,50,51,52,53,54,55,57,59,64,67,73,74,78) THEN 1
			WHEN V.ItemLogCode IN (20,21,22,23,24,25,26,27,30,38,42,56,58,77) THEN 2
			WHEN V.ItemLogCode = 12 AND V.ItemLogFKey1 > 1000 THEN 4
			WHEN V.ItemLogCode = 12 AND (V.ItemLogFKey1 = 0 OR V.ItemLogFKey1 IS NULL) THEN 5 END
		, CASE WHEN V.ItemOption >= il.ItemOption1Start AND V.ItemOption <= il.ItemOption1End THEN 1
			WHEN V.ItemOption >= il.ItemOption2Start AND V.ItemOption <= il.ItemOption2End THEN 2
			WHEN V.ItemOption >= il.ItemOption3Start AND V.ItemOption <= il.ItemOption3End THEN 3
			ELSE 0 END
		, CASE WHEN dc.LastLoginDate >= DATEADD(DD, -31, getdate()) THEN 1 ELSE 0 END, il.ItemCategoryID
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
