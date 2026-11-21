USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildDepotItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 20
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-08

description :
	?? ?? ??? ??? ?????. (??!!! ???? ?? GuildDepotSize? ??? ? ????.)
result set :
	@intVersion < 11 : GuildDepotIndex/smallint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime
	@intVersion = 11 : GuildDepotIndex/smallint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, ItemPotentialMoveCount/tinyint
	@intVersion = 12 : Version 11 + DragonJewelType/tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ?? ??? ?? ?? ?? ?? at 2010-10-11
	3 = Slot ??? ??? ???? ?? ?? ??? ?? -1? ????? ?? at 2010-10-12
	4 = ?? ?? ?? ?? at 2010-10-12
	5 = Slot ??? ??? ???? ?? ?? ??? ?? 0?? ????? ?? at 2010-10-12
	10 = LoopJoin ?? ???? ?? ?? ?? by ??? at 2012-03-22
	11~12 = ?? ?? ?? ?? (@intVersion = 11)     by ??? at 2012-04-12
	13 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-18
	14 = ?????? ???? ??? ???? ?? ??(NULL ?? ???? ???) by ??? at 2012-06-21
	15 = ?? ???? ???? ?? ??? ??? ??? ???? ?? ?? ?? by ??? at 2012-11-07
	16 = ?? ??? ?? by ??? at 2012-12-27
	18 = ???? MaterializedItemID ?? ?? ?? by ??? at 2012-12-27
	19 = ?? ?? ??, ??? ?? by ??? at 2012-12-27
	20 = ?? ?? ?? ?? by ??? at 2013-04-04
**/

CREATE PROCEDURE [dbo].[P_GetListGuildDepotItem] 
	  @intGuildID int --// ?? ID
	, @intVersion int = 10 --// ??? SP? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ?? ?? ???? ?????*/
	IF @intVersion < 11
	BEGIN
		SELECT ISNULL(GDS.GuildDepotIndex, 0) AS GuildDepotIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability
			, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption
			, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
		FROM dbo.GuildDepots GD WITH (NOLOCK, INDEX(PK_GuildDepots), FORCESEEK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(PK_MaterializedItems), FORCESEEK) ON GD.MaterializedItemID = MI.MaterializedItemID
			LEFT OUTER JOIN dbo.GuildDepotSlots GDS WITH (NOLOCK, INDEX(PK_GuildDepotSlots), FORCESEEK) ON GD.GuildID = GDS.GuildID AND GD.ItemSerial = GDS.ItemSerial
		WHERE GD.GuildID = @intGuildID
		OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
	END
	ELSE IF @intVersion = 11
	BEGIN
		SELECT ISNULL(GDS.GuildDepotIndex, 0) AS GuildDepotIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability
			, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption
			, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, MI.ItemPotentialMoveCount
		FROM dbo.GuildDepots GD WITH (NOLOCK, INDEX(PK_GuildDepots), FORCESEEK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(PK_MaterializedItems), FORCESEEK) ON GD.MaterializedItemID = MI.MaterializedItemID
			LEFT OUTER JOIN dbo.GuildDepotSlots GDS WITH (NOLOCK, INDEX(PK_GuildDepotSlots), FORCESEEK) ON GD.GuildID = GDS.GuildID AND GD.ItemSerial = GDS.ItemSerial
		WHERE GD.GuildID = @intGuildID
		OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
	END
	ELSE IF @intVersion = 12
	BEGIN
		SELECT ISNULL(GDS.GuildDepotIndex, 0) AS GuildDepotIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability
			, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption
			, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, MI.ItemPotentialMoveCount, MI.DragonJewelType
		FROM dbo.GuildDepots GD WITH (NOLOCK, INDEX(PK_GuildDepots), FORCESEEK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(PK_MaterializedItems), FORCESEEK) ON GD.MaterializedItemID = MI.MaterializedItemID
			LEFT OUTER JOIN dbo.GuildDepotSlots GDS WITH (NOLOCK, INDEX(PK_GuildDepotSlots), FORCESEEK) ON GD.GuildID = GDS.GuildID AND GD.ItemSerial = GDS.ItemSerial
		WHERE GD.GuildID = @intGuildID
		OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
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
