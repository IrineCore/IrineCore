USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListEventRewardItem]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListEventRewardItem]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListEventRewardItem] AS' 
END
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-08-23

description :
	??? ?? ??? ?? ??
result set : 
	version = 1
		RewardItemSequenceNumber / tinyint / ?? ??? ?? ??
		ProductFlag / bit / ?? ?? (0=??, 1=??)
		ItemID / bigint / ??? ID (ProductFlag=1 ? ???? S/N)
		ItemCount / smallint / (ProductFlag=0 ? ?? ??) ??
		ItemDurability / smallint / (ProductFlag=0 ? ?? ??) ???
		RandomSeed / int / (ProductFlag=0 ? ?? ??) ????
		ItemLevel / tinyint / (ProductFlag=0 ? ?? ??) ????
		ItemPotential / tinyint / (ProductFlag=0 ? ?? ??) ???
		SoulBoundFlag / bit / (ProductFlag=0 ? ?? ??) ???? (0=??, 1=??)
		SealCount / tinyint / (ProductFlag=0 ? ?? ??) ?? ??? ??
		ItemOption / tinyint / (ProductFlag=0 ? ?? ??) ??? ??
		ItemLifespan / int / (ProductFlag=0 ? ?? ??) ??? ?? (?)
		EternityFlag / bit / (ProductFlag=0 ? ?? ??) ?? ??? ?? (0=???, 1=??)
		ItemExpireDate / smalldatetime / (ProductFlag=0 ? ?? ??) ??? ?? ??
	version = 2
		RewardItemSequenceNumber / tinyint / ?? ??? ?? ??
		ProductFlag / bit / ?? ?? (0=??, 1=??)
		ItemID / bigint / ??? ID (ProductFlag=1 ? ???? S/N)
		ItemCount / smallint / (ProductFlag=0 ? ?? ??) ??
		ItemDurability / smallint / (ProductFlag=0 ? ?? ??) ???
		RandomSeed / int / (ProductFlag=0 ? ?? ??) ????
		ItemLevel / tinyint / (ProductFlag=0 ? ?? ??) ????
		ItemPotential / tinyint / (ProductFlag=0 ? ?? ??) ???
		SoulBoundFlag / bit / (ProductFlag=0 ? ?? ??) ???? (0=??, 1=??)
		SealCount / tinyint / (ProductFlag=0 ? ?? ??) ?? ??? ??
		ItemOption / tinyint / (ProductFlag=0 ? ?? ??) ??? ??
		ItemLifespan / int / (ProductFlag=0 ? ?? ??) ??? ?? (?)
		EternityFlag / bit / (ProductFlag=0 ? ?? ??) ?? ??? ?? (0=???, 1=??)
		ItemExpireDate / smalldatetime / (ProductFlag=0 ? ?? ??) ??? ?? ??
		DragonJewelType / tinyint / (ProductFlag=0 ? ?? ??) ??????
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101201 = ??? ?? ???? ????.
**/
ALTER PROCEDURE [dbo].[P_GetListEventRewardItem]
	  @intEventRewardID int --// ??? ?? ID
	, @intVersion int = 1 --// ??
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
	/**_# ??? ?? ??? ?? ?? */
	IF @intVersion = 1
		SELECT RewardItemSequenceNumber
			 , ProductFlag
			 , ItemID
			 , ItemCount
			 , ItemDurability
			 , RandomSeed
			 , ItemLevel
			 , ItemPotential
			 , SoulBoundFlag
			 , SealCount
			 , ItemOption
			 , ItemLifespan
			 , EternityFlag
			 , ItemExpireDate
		  FROM dbo.EventRewardItems WITH (NOLOCK)
		 WHERE EventRewardID = @intEventRewardID
		OPTION (KEEPFIXED PLAN);
	ELSE IF @intVersion = 2
		SELECT RewardItemSequenceNumber
			 , ProductFlag
			 , ItemID
			 , ItemCount
			 , ItemDurability
			 , RandomSeed
			 , ItemLevel
			 , ItemPotential
			 , SoulBoundFlag
			 , SealCount
			 , ItemOption
			 , ItemLifespan
			 , EternityFlag
			 , ItemExpireDate
			 , DragonJewelType
		  FROM dbo.EventRewardItems WITH (NOLOCK)
		 WHERE EventRewardID = @intEventRewardID
		OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101201;
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
