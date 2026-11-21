USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCharacterStarterItem]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-08
description : ??? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddCharacterStarterItem]
	  @intItemID int --// ?? ???ID
	, @inyItemLocationCode tinyint --// ??? ?? ??
	, @inyItemLocationIndex tinyint --// ??? ?? ???
	, @sdtProvideStartDate smalldatetime = NULL --// ?? ?? ??
	, @sdtProvideEndDate smalldatetime = NULL --// ?? ?? ??	
	, @insItemCount	smallint --// ??? ??
	, @insItemDurability smallint --// ??? ???
	, @intRandomSeed int --// ????
	, @intCoolTime	int --// ? ?? (?)
	, @inyItemLevel	tinyint --// ????
	, @inyItemPotential	tinyint --// ??? 
	, @bitSoulBoundFlag	bit --// 0=??, 1=?????
	, @inySealCount	tinyint --// ?? ???
	, @inyItemOption	tinyint --// ??? ??
	, @intItemLifespan	int --// ??? ?? (?)
	, @bitEternityFlag	bit --// 0=???? ?? ??, 1=?? ???
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
	/**_# [CharacterStarterItems] ???? INSERT???.*/
	INSERT dbo.CharacterStarterItems (ItemID, ItemLocationCode, ItemLocationIndex, ProvideStartDate, ProvideEndDate, RegisterDate, ItemCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag)
	VALUES (@intItemID, @inyItemLocationCode, @inyItemLocationIndex, @sdtProvideStartDate, @sdtProvideEndDate, GETDATE(), @insItemCount, @insItemDurability, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @intItemLifespan, @bitEternityFlag);	
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
