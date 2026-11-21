USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelCashIItem]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-01-26
description : ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103202 = ??? ???? ???? ????.
history : 
3~4 = ??? ?? ??. by dailykim at 2012-05-30
**/

CREATE PROCEDURE [dbo].[P_DelCashIItem]
	  @inbItemSerial bigint --// ??? ???? ??? ???
	, @insItemRemainCount smallint --// ??? ??
	, @intWaitingTime int --// ?? ?? ??. ?? : ?
	, @sdtItemExpireDate smalldatetime OUTPUT --// ?? ???? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
		, @intItemID int
		, @inbOwnerCharacterID bigint
		, @dt2OwnershipStartDate datetime2(0)
		, @insItemCount smallint
		, @dt2Now datetime2(0) = GETDATE()
		, @bitSoulBoundFlag bit
		, @inySealCount tinyint
		, @inyItemOption tinyint
		, @inyItemPotential tinyint
		, @insRemainItemCount smallint
		, @insItemRemainCountBefore smallint
		, @inyItemLevel tinyint
		, @inyItemPotentialMoveCount tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ??? ??? ??? ??? ?????.*/
	SELECT @intItemID = ItemID
		     , @inbOwnerCharacterID = OwnerCharacterID
		     , @dt2OwnershipStartDate = OwnershipStartDate
		     , @insItemRemainCountBefore = ItemRemainCount
		     , @insRemainItemCount = 0
		     , @bitSoulBoundFlag = SoulBoundFlag
		     , @inySealCount = SealCount
		     , @inyItemOption = ItemOption
		     , @inyItemPotential = ItemPotential
			 , @inyItemPotentialMoveCount = ItemPotentialMoveCount
			 , @inyItemLevel = ItemLevel
	FROM dbo.MaterializedItems WITH (NOLOCK)
	WHERE ItemSerial = @inbItemSerial;

	UPDATE dbo.MaterializedItems
	SET @sdtItemExpireDate = ItemExpireDate = 
			CASE
				WHEN EternityFlag = 0 AND ItemExpireDate < DATEADD(MINUTE, @intWaitingTime, GETDATE()) THEN ItemExpireDate
				ELSE DATEADD(MINUTE, @intWaitingTime, GETDATE())
			END
		, ExpireCompleteFlag = 1
		, EternityFlag = 0
		, ItemLifespan = CASE WHEN EternityFlag = 0 THEN ItemLifespan ELSE 2628000 END
	WHERE ItemSerial = @inbItemSerial
		AND ItemLocationCode = 5
		AND ItemRemainCount = @insItemRemainCount
		AND ExpireCompleteFlag = 0;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103202;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ?????.*/
	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 65
		, @inbOwnerCharacterID = NULL
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = 0
		, @insItemCountBefore = @insItemRemainCountBefore
		, @inbItemLogFKey1 = NULL
		, @inbItemLogFKey2 = @intWaitingTime
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemLevel = @inyItemLevel;

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
