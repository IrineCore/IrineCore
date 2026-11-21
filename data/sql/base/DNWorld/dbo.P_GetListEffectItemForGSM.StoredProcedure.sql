USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListEffectItemForGSM]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-16
description : ???? ??? ?? ??? ??? ?????. (GSM ???)
result set : EffectItemID/int, ItemID/int, EffectItemGetCode/tinyint, EffectItemGetKey/bigint, EternityFlag/bit, EffectItemExpireDate/smalldatetime, Property1/int, Property2/int, Property3/int, Property4/int, Property5/int, RegisterDate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103187 = ??? ?? ???? ????.
history :
2 = ??? ??? ?? ??? ??? SP? ?? at 2010-06-30
3 = EffectItemID ??? ????? ?? at 2010-08-20
4 = ??? ?? ?? by ??? at 2013-01-16
**/

CREATE PROCEDURE [dbo].[P_GetListEffectItemForGSM]
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
	/**_# ??? ?? ??? ??? ?????.*/
	SELECT EffectItemID, ItemID, EffectItemGetCode, EffectItemGetKey, EternityFlag, EffectItemExpireDate, Property1, Property2, Property3, Property4, Property5, RegisterDate
	FROM dbo.EffectItems WITH (NOLOCK, INDEX(IX_NC_EffectItems_CharacterID), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION(KEEPFIXED PLAN);

	/**_# ??? ?? ???? ??? 103187? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103187;
		GOTO ErrorHandler;
	END
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
