USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UpsChaosGaugeItem]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-03-22
description : ??? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_UpsChaosGaugeItem]
	  @inbCharacterID bigint --// ???ID
	, @intChaosGaugeType int --// ????????
	, @intItemGauge int --// ???
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
	/**_# [ChaosGaugeItems] ???? UPSERT???.*/
	UPDATE dbo.ChaosGaugeItems
	SET ItemGauge = @intItemGauge
	WHERE CharacterID = @inbCharacterID
	  AND ChaosGaugeType = @intChaosGaugeType

	IF @@ROWCOUNT = 0
		INSERT dbo.ChaosGaugeItems (CharacterID, ChaosGaugeType, ItemGauge)
		VALUES (@inbCharacterID, @intChaosGaugeType, @intItemGauge)

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
