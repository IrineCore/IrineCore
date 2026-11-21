USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddParty]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 21
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-05
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
10 = MaxLevel ?? ??? ?? ?? by ??? at 2012-03-07
11~12 = ???? ??? ?? ??? ?? ?? by ??? at 2012-03-12
13 = PartyCode ?? ?? by ??? at 2012-03-26
14 = RoomID ?? ? ?? ?? by ??? at 2012-03-27
history :
15 = ?? ??? ??? ???? ???? ??? ?? by ??? at 2012-05-11
16 = ?? ?? ?? ?? by ??? at 2012-05-14
18 = ??? ?? ?? by ??? at 2012-08-24
**/

CREATE PROCEDURE [dbo].[P_AddParty]
	  @intServerID int --// ??ID
	, @intWorldID int --// ??ID
	, @intRoomID int --// ? ID(???)
	, @inbCharacterID bigint --//??? ???? ???ID
	, @inyServerType tinyint --// ??? ??
	, @intServerCommonID int --// ??? ??? ??ID
	, @inyPartyCode tinyint --// 0=????, 1=?????, 2=????
	, @nvcPartyName nvarchar(40) --// ????
	, @insPartyMaxCount smallint --// ???? ???
	, @intGoalMapID int --// ?? ????ID
	, @inyDifficultyCode tinyint --// 1=??,2??,3=???,4=???,5=???
	, @intPartyOptionFlagBitmap int --// ?? ?? ??? ?
	, @intPassWordValue int --// ??????(??4??)
	, @inyMinLevel tinyint --// ?? ??
	, @inyDefaultDistributionCode tinyint --// 0=??,1=????,2=?????,3=??????,4=???????
	, @inyRandomDistributionCode tinyint --// 0=??,1=??,2=??,3=??,4=???,5=???,6=??????
	, @intUpkeepCount int --// ?? ?? ???
	, @inbPartyID bigint = NULL OUTPUT --// ??ID	
	, @inyCharacterTypeCode tinyint = NULL --// 0=?????, 1=??????, 2=????, 3=??????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON; 
SET XACT_ABORT ON;
 
DECLARE @intReturnValue int;
DECLARE @inbGuildID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION

	/**_# [Parties] ???? INSERT???.*/
	INSERT dbo.Parties (ServerID, WorldID, RoomID, ServerType, ServerCommonID, PartyCode, PartyName, PartyMaxCount, GoalMapID, DifficultyCode, PartyOptionFlagBitmap, PassWordValue, MinLevel, DefaultDistributionCode, RandomDistributionCode, UpkeepCount)
	VALUES (@intServerID, @intWorldID, @intRoomID, @inyServerType, @intServerCommonID, @inyPartyCode, @nvcPartyName, @insPartyMaxCount, @intGoalMapID, @inyDifficultyCode, @intPartyOptionFlagBitmap, @intPassWordValue, @inyMinLevel, @inyDefaultDistributionCode, @inyRandomDistributionCode, @intUpkeepCount);

	SET @inbPartyID = SCOPE_IDENTITY();

	/**_# ????? ?? ???? ???? ?????.*/
	IF @inyPartyCode <> 2
	BEGIN
		/**_# [PartyJoins] ???? INSERT???.*/
		INSERT dbo.PartyJoins (PartyID, CharacterID, PartyOwnerFlag, CharacterTypeCode)
		VALUES (@inbPartyID, @inbCharacterID, CAST(1 AS bit), ISNULL(@inyCharacterTypeCode, 0));
	END
	COMMIT TRANSACTION

	/**_# [PartyStartIDs] ???? INSERT???.*/
	INSERT dbo.PartyStartIDs (PartyID, PartyStartDate, PartyEndDate)
	VALUES (@inbPartyID, GETDATE(), NULL);

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 8115
	BEGIN
		DBCC CHECKIDENT(PartyJoins, reseed, 1) WITH NO_INFOMSGS 
		EXEC @intReturnValue = dbo.P_AddParty
			 @intServerID = @intServerID
		   , @intWorldID = @intWorldID
		   , @intRoomID = @intWorldID
		   , @inbCharacterID = @inbCharacterID
		   , @inyServerType = @inyServerType
		   , @intServerCommonID = @intServerCommonID
		   , @inyPartyCode = @inyPartyCode
		   , @nvcPartyName = @nvcPartyName
		   , @insPartyMaxCount = @insPartyMaxCount
		   , @intGoalMapID = @intGoalMapID
		   , @inyDifficultyCode = @inyDifficultyCode
		   , @intPartyOptionFlagBitmap = @intPartyOptionFlagBitmap
		   , @intPassWordValue = @intPassWordValue
		   , @inyMinLevel = @inyMinLevel
		   , @inyDefaultDistributionCode = @inyDefaultDistributionCode
		   , @inyRandomDistributionCode = @inyRandomDistributionCode
		   , @intUpkeepCount = @intUpkeepCount
		   , @inbPartyID = @inbPartyID OUTPUT;
	END	
	ELSE
	BEGIN
		GOTO ErrorHandler;
	END
END CATCH;
RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
