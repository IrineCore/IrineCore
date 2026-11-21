USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_ST_DailyItemCount]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 14
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-09
description : (TIMOSE) ??? ??? ?? SSIS Snapshot (dbo.ST_DailyItemCount)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~2 = ???? by ??? at 2013-04-09
3 = ItemLocationCode ?? by ??? at 2013-04-26
4~7 = JobCode ??, CharacterLevel & ItemLevel ???, ItemID->ItemCategoryID ?? by ??? at 2013-04-29
10~11 = ?? ?? (Characters ??? WorldCoverage? ??, ????->?????? ??) by ??? at 2013-05-13
12 = ItemLocationCode ?? by ??? at 2013-05-13
13 = dbo. ??? ?? ?? by ??? at 2013-05-22
14 = ?? ??? ?? ?? by ??? at 2013-07-05
**/

CREATE PROCEDURE [dbo].[P_TMS_ST_DailyItemCount]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP 1 @intWorldID = WorldID FROM dbo.WorldCoverage WITH(NOLOCK);

	/** ??? ?? ?? **/
	IF OBJECT_ID('tempdb..#ST_DailyItemCount') IS NOT NULL DROP TABLE #ST_DailyItemCount;
	SELECT CAST(DATEADD(DD, -1, GETDATE()) AS date) AS BasisDate, @intWorldID AS WorldID
		, CASE WHEN cs.CharacterLevel >= 1 AND cs.CharacterLevel <= 5 THEN 1 WHEN cs.CharacterLevel >= 6 AND cs.CharacterLevel <= 10 THEN 2
			WHEN cs.CharacterLevel >= 11 AND cs.CharacterLevel <= 15 THEN 3 WHEN cs.CharacterLevel >= 16 AND cs.CharacterLevel <= 24 THEN 4
			WHEN cs.CharacterLevel >= 25 AND cs.CharacterLevel <= 32 THEN 5 WHEN cs.CharacterLevel >= 33 AND cs.CharacterLevel <= 39 THEN 6
			WHEN cs.CharacterLevel = 40 THEN 7 WHEN cs.CharacterLevel >= 41 AND cs.CharacterLevel <= 49 THEN 8
			WHEN cs.CharacterLevel = 50 THEN 9 WHEN cs.CharacterLevel >= 51 AND cs.CharacterLevel <= 59 THEN 10
			WHEN cs.CharacterLevel = 60 THEN 11 WHEN cs.CharacterLevel >= 61 AND cs.CharacterLevel <= 69 THEN 12
			WHEN cs.CharacterLevel = 70 THEN 13 WHEN cs.CharacterLevel >= 71 AND cs.CharacterLevel <= 79 THEN 14
			WHEN cs.CharacterLevel = 80 THEN 15 WHEN cs.CharacterLevel >= 81 AND cs.CharacterLevel <= 89 THEN 16
			WHEN cs.CharacterLevel = 90 THEN 17 WHEN cs.CharacterLevel >= 91 AND cs.CharacterLevel <= 99 THEN 18
			WHEN cs.CharacterLevel = 100 THEN 19 END AS CharacterLevel, NULL AS JobCode, NULL AS ItemID
		, CASE WHEN i.ItemLevel > 0 AND i.ItemLevel < 7 THEN 6 ELSE i.ItemLevel END AS ItemLevel, 3 AS ItemUseCode
		, CASE WHEN i.ItemOption >= il.ItemOption1Start AND i.ItemOption <= il.ItemOption1End THEN 1
			WHEN i.ItemOption >= il.ItemOption2Start AND i.ItemOption <= il.ItemOption2End THEN 2
			WHEN i.ItemOption >= il.ItemOption3Start AND i.ItemOption <= il.ItemOption3End THEN 3 ELSE 0 END AS ItemOptionGroupCode
		, CASE WHEN cs.LastLoginDate >= DATEADD(DD, -31, getdate()) THEN 1 ELSE 0 END AS ActiveUserFlag
		, i.ItemLocationCode
		, SUM(ISNULL(i.ItemCount, 1)) AS ItemCount, il.ItemCategoryID
	INTO #ST_DailyItemCount
	FROM dbo.ItemListForCount il WITH (NOLOCK)
	INNER JOIN dbo.MaterializedItems i WITH (NOLOCK) ON i.ItemID = il.ItemID
	INNER JOIN dbo.CharacterStatus cs WITH (NOLOCK) ON cs.CharacterID = i.OwnerCharacterID
	GROUP BY CASE WHEN cs.CharacterLevel >= 1 AND cs.CharacterLevel <= 5 THEN 1 WHEN cs.CharacterLevel >= 6 AND cs.CharacterLevel <= 10 THEN 2
			WHEN cs.CharacterLevel >= 11 AND cs.CharacterLevel <= 15 THEN 3 WHEN cs.CharacterLevel >= 16 AND cs.CharacterLevel <= 24 THEN 4
			WHEN cs.CharacterLevel >= 25 AND cs.CharacterLevel <= 32 THEN 5 WHEN cs.CharacterLevel >= 33 AND cs.CharacterLevel <= 39 THEN 6
			WHEN cs.CharacterLevel = 40 THEN 7 WHEN cs.CharacterLevel >= 41 AND cs.CharacterLevel <= 49 THEN 8
			WHEN cs.CharacterLevel = 50 THEN 9 WHEN cs.CharacterLevel >= 51 AND cs.CharacterLevel <= 59 THEN 10
			WHEN cs.CharacterLevel = 60 THEN 11 WHEN cs.CharacterLevel >= 61 AND cs.CharacterLevel <= 69 THEN 12
			WHEN cs.CharacterLevel = 70 THEN 13 WHEN cs.CharacterLevel >= 71 AND cs.CharacterLevel <= 79 THEN 14
			WHEN cs.CharacterLevel = 80 THEN 15 WHEN cs.CharacterLevel >= 81 AND cs.CharacterLevel <= 89 THEN 16
			WHEN cs.CharacterLevel = 90 THEN 17 WHEN cs.CharacterLevel >= 91 AND cs.CharacterLevel <= 99 THEN 18
			WHEN cs.CharacterLevel = 100 THEN 19 END
		, CASE WHEN i.ItemLevel > 0 AND i.ItemLevel < 7 THEN 6 ELSE i.ItemLevel END
		, CASE WHEN i.ItemOption >= il.ItemOption1Start AND i.ItemOption <= il.ItemOption1End THEN 1
			WHEN i.ItemOption >= il.ItemOption2Start AND i.ItemOption <= il.ItemOption2End THEN 2
			WHEN i.ItemOption >= il.ItemOption3Start AND i.ItemOption <= il.ItemOption3End THEN 3 ELSE 0 END
		, CASE WHEN cs.LastLoginDate >= DATEADD(DD, -31, getdate()) THEN 1 ELSE 0 END, i.ItemLocationCode, il.ItemCategoryID
	OPTION (MAXDOP 1, KEEPFIXED PLAN);
	
	/** ?? ?? **/
	IF OBJECT_ID('tempdb..#ST_DailyItemCount2') IS NOT NULL DROP TABLE #ST_DailyItemCount2;
	SELECT CAST(DATEADD(DD, -1, GETDATE()) AS date) AS BasisDate, @intWorldID AS WorldID
		, CASE WHEN cs.CharacterLevel >= 1 AND cs.CharacterLevel <= 5 THEN 1 WHEN cs.CharacterLevel >= 6 AND cs.CharacterLevel <= 10 THEN 2
			WHEN cs.CharacterLevel >= 11 AND cs.CharacterLevel <= 15 THEN 3 WHEN cs.CharacterLevel >= 16 AND cs.CharacterLevel <= 24 THEN 4
			WHEN cs.CharacterLevel >= 25 AND cs.CharacterLevel <= 32 THEN 5 WHEN cs.CharacterLevel >= 33 AND cs.CharacterLevel <= 39 THEN 6
			WHEN cs.CharacterLevel = 40 THEN 7 WHEN cs.CharacterLevel >= 41 AND cs.CharacterLevel <= 49 THEN 8
			WHEN cs.CharacterLevel = 50 THEN 9 WHEN cs.CharacterLevel >= 51 AND cs.CharacterLevel <= 59 THEN 10
			WHEN cs.CharacterLevel = 60 THEN 11 WHEN cs.CharacterLevel >= 61 AND cs.CharacterLevel <= 69 THEN 12
			WHEN cs.CharacterLevel = 70 THEN 13 WHEN cs.CharacterLevel >= 71 AND cs.CharacterLevel <= 79 THEN 14
			WHEN cs.CharacterLevel = 80 THEN 15 WHEN cs.CharacterLevel >= 81 AND cs.CharacterLevel <= 89 THEN 16
			WHEN cs.CharacterLevel = 90 THEN 17 WHEN cs.CharacterLevel >= 91 AND cs.CharacterLevel <= 99 THEN 18
			WHEN cs.CharacterLevel = 100 THEN 19 END AS CharacterLevel, NULL AS JobCode, NULL AS ItemID
		, CASE WHEN i.ItemLevel > 0 AND i.ItemLevel < 7 THEN 6 ELSE i.ItemLevel END AS ItemLevel, 3 AS ItemUseCode
		, CASE WHEN i.ItemOption >= il.ItemOption1Start AND i.ItemOption <= il.ItemOption1End THEN 1
			WHEN i.ItemOption >= il.ItemOption2Start AND i.ItemOption <= il.ItemOption2End THEN 2
			WHEN i.ItemOption >= il.ItemOption3Start AND i.ItemOption <= il.ItemOption3End THEN 3 ELSE 0 END AS ItemOptionGroupCode
		, CASE WHEN cs.LastLoginDate >= DATEADD(DD, -31, getdate()) THEN 1 ELSE 0 END AS ActiveUserFlag
		, CASE WHEN ip.ItemLocationCode = 1 THEN 24 ELSE 25 END AS ItemLocationCode 
		, SUM(ISNULL(i.ItemCount, 1)) AS ItemCount, il.ItemCategoryID
	INTO #ST_DailyItemCount2
	FROM dbo.ItemListForCount il
	INNER JOIN dbo.DragonJewels dj ON dj.ItemID = il.ItemID
	INNER JOIN dbo.MaterializedItems ip WITH (NOLOCK) ON ip.MaterializedItemID = dj.ParentItemMaterializedItemID
	INNER JOIN dbo.MaterializedItems i WITH (NOLOCK) ON i.ItemSerial = dj.ItemSerial
	INNER JOIN dbo.CharacterStatus cs WITH (NOLOCK) ON cs.CharacterID = ip.OwnerCharacterID
	GROUP BY CASE WHEN cs.CharacterLevel >= 1 AND cs.CharacterLevel <= 5 THEN 1 WHEN cs.CharacterLevel >= 6 AND cs.CharacterLevel <= 10 THEN 2
			WHEN cs.CharacterLevel >= 11 AND cs.CharacterLevel <= 15 THEN 3 WHEN cs.CharacterLevel >= 16 AND cs.CharacterLevel <= 24 THEN 4
			WHEN cs.CharacterLevel >= 25 AND cs.CharacterLevel <= 32 THEN 5 WHEN cs.CharacterLevel >= 33 AND cs.CharacterLevel <= 39 THEN 6
			WHEN cs.CharacterLevel = 40 THEN 7 WHEN cs.CharacterLevel >= 41 AND cs.CharacterLevel <= 49 THEN 8
			WHEN cs.CharacterLevel = 50 THEN 9 WHEN cs.CharacterLevel >= 51 AND cs.CharacterLevel <= 59 THEN 10
			WHEN cs.CharacterLevel = 60 THEN 11 WHEN cs.CharacterLevel >= 61 AND cs.CharacterLevel <= 69 THEN 12
			WHEN cs.CharacterLevel = 70 THEN 13 WHEN cs.CharacterLevel >= 71 AND cs.CharacterLevel <= 79 THEN 14
			WHEN cs.CharacterLevel = 80 THEN 15 WHEN cs.CharacterLevel >= 81 AND cs.CharacterLevel <= 89 THEN 16
			WHEN cs.CharacterLevel = 90 THEN 17 WHEN cs.CharacterLevel >= 91 AND cs.CharacterLevel <= 99 THEN 18
			WHEN cs.CharacterLevel = 100 THEN 19 END
		, CASE WHEN i.ItemLevel > 0 AND i.ItemLevel < 7 THEN 6 ELSE i.ItemLevel END
		, CASE WHEN i.ItemOption >= il.ItemOption1Start AND i.ItemOption <= il.ItemOption1End THEN 1
			WHEN i.ItemOption >= il.ItemOption2Start AND i.ItemOption <= il.ItemOption2End THEN 2
			WHEN i.ItemOption >= il.ItemOption3Start AND i.ItemOption <= il.ItemOption3End THEN 3 ELSE 0 END
		, CASE WHEN cs.LastLoginDate >= DATEADD(DD, -31, getdate()) THEN 1 ELSE 0 END
		, CASE WHEN ip.ItemLocationCode = 1 THEN 24 ELSE 25 END
		, il.ItemCategoryID
	OPTION (MAXDOP 1, KEEPFIXED PLAN);
	
	SELECT BasisDate, WorldID, CharacterLevel, JobCode, ItemID, ItemLevel, ItemUseCode, ItemOptionGroupCode, ActiveUserFlag, ItemLocationCode, ItemCount, ItemCategoryID
	FROM #ST_DailyItemCount
	UNION ALL
	SELECT BasisDate, WorldID, CharacterLevel, JobCode, ItemID, ItemLevel, ItemUseCode, ItemOptionGroupCode, ActiveUserFlag, ItemLocationCode, ItemCount, ItemCategoryID
	FROM #ST_DailyItemCount2
	OPTION (MAXDOP 1);
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
