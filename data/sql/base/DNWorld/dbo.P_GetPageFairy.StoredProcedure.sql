USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageFairy]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-02
description : ???? ??? ??????.
result set : FairyItemSerial/bigint, FairyExp/int, FairyStatus1/int, FairyStatus2/int, FairyStatus3/int, FairyStatus4/int, FairyStatus5/int, FairyStatus6/int, ShapeID/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetPageFairy]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @inbCharacterID bigint --// ??? ID
	, @intTotalCount int OUTPUT --// ?? ?? ?? ?????. (1?????? ?????.)
	, @bitEquipmentFlag bit = 0 --// ?? ?? (0=??? ?? ??, 1=??? ??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intPageNumber = 1 AND (@bitEquipmentFlag = 0 OR @bitEquipmentFlag IS NULL)
		EXEC @intReturnValue = dbo.P_GetCountFairy		  
			 @inbCharacterID = @inbCharacterID		   
		   , @intTotalCount = @intTotalCount OUTPUT;
	
	/**_# [Fairies] ???? SELECT???.*/
	IF @bitEquipmentFlag = 1
		WITH C1 AS (
			SELECT ROW_NUMBER() OVER(ORDER BY F.FairyItemSerial) AS Seq, F.FairyItemSerial, F.FairyExp, F.FairyStatus1, F.FairyStatus2, F.FairyStatus3, F.FairyStatus4, F.FairyStatus5, F.FairyStatus6, F.ShapeID
			FROM dbo.Fairies F WITH (NOLOCK, INDEX(IX_NN_Fairies_OwnerCharacterID), FORCESEEK)
				 INNER JOIN dbo.MaterializedItems M WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON F.FairyItemSerial = M.ItemSerial
			WHERE F.OwnerCharacterID = @inbCharacterID AND M.ItemLocationCode = 7
		)
		SELECT FairyItemSerial, FairyExp, FairyStatus1, FairyStatus2, FairyStatus3, FairyStatus4, FairyStatus5, FairyStatus6, ShapeID
		FROM C1
		WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
		ORDER BY Seq
		OPTION (KEEPFIXED PLAN);			
	ELSE
		WITH C1 AS (
			SELECT ROW_NUMBER() OVER(ORDER BY F.FairyItemSerial) AS Seq, F.FairyItemSerial, F.FairyExp, F.FairyStatus1, F.FairyStatus2, F.FairyStatus3, F.FairyStatus4, F.FairyStatus5, F.FairyStatus6, F.ShapeID
			FROM dbo.Fairies F WITH (NOLOCK, INDEX(IX_NN_Fairies_OwnerCharacterID), FORCESEEK)				 
			WHERE F.OwnerCharacterID = @inbCharacterID
		)
		SELECT FairyItemSerial, FairyExp, FairyStatus1, FairyStatus2, FairyStatus3, FairyStatus4, FairyStatus5, FairyStatus6, ShapeID
		FROM C1
		WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
		ORDER BY Seq
		OPTION (KEEPFIXED PLAN);
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
