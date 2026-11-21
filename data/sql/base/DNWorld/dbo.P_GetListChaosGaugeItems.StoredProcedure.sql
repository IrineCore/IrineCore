USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListChaosGaugeItems]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-03-22
description : ??? ??? ???? ?????.
result set : CharacterID/bigint, ChaosGaugeType/int, ItemGauge/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103393 = ?????? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListChaosGaugeItems]
	  @inbCharacterID bigint --// ???ID
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
	/**_# [ChaosGaugeItems] ???? SELECT???.*/
	SELECT CharacterID, ChaosGaugeType, ItemGauge
	FROM dbo.ChaosGaugeItems WITH(NOLOCK, INDEX(PK_ChaosGaugeItems), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103393;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
