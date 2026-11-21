USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListManufactureSkillRecipe]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-03
description : ?? ?? ??? ??? ??? ?????.
result set : SecondarySkillID/int, ItemSerial/bigint, ItemID/int, ItemDurability/smallint, ItemLifespan/int, EternityFlag/bit, ItemExpireDate/smalldatetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5=Tuning     by ??? at 2012-03-19
**/

CREATE PROCEDURE [dbo].[P_GetListManufactureSkillRecipe]
	  @inbCharacterID bigint --// ??? ID
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
	SELECT MSR.SecondarySkillID, MI.ItemSerial, MI.ItemID, MI.ItemDurability, MI.ItemLifespan, MI.EternityFlag, MI.ItemExpireDate
	FROM dbo.ManufactureSkillRecipes MSR WITH (NOLOCK, INDEX(PK_ManufactureSkillRecipes), FORCESEEK)
		INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON MSR.ItemSerial = MI.ItemSerial
	WHERE MSR.CharacterID = @inbCharacterID
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
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
