USE [dnmembership]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[P_AddGuildWarMakeGiftQueue]
	  @inbCharacterID bigint
	, @bitPCRoomFlag bit
	, @intProductID int
	, @vchSelectedItemIDs varchar(8000)
	, @nvcGiftMemo nvarchar(100)
	, @intLifespan int
	, @inyPayMethodCode tinyint
	, @intMatchTypeCode int
	, @vchIP varchar(15)
	, @vchItemOptions varchar(8000) = ''
	, @bitNewFlag bit = 0
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @inbPurchaseOrderID bigint
	  , @vchOrderKey varchar(32) = CAST(@intMatchTypeCode AS varchar(32))
	  , @vchCleanOptions varchar(8000) = CASE WHEN @vchItemOptions = '0' THEN '' ELSE ISNULL(@vchItemOptions, '') END;

EXEC @intReturnValue = dbo.P_MakeGift
	  @inbCharacterID = @inbCharacterID
	, @bitPCRoomFlag = @bitPCRoomFlag
	, @intProductID = @intProductID
	, @vchSelectedItemIDs = @vchSelectedItemIDs
	, @nvcGiftMemo = @nvcGiftMemo
	, @intLifespan = @intLifespan
	, @inyPayMethodCode = @inyPayMethodCode
	, @vchOrderKey = @vchOrderKey
	, @vchIP = @vchIP
	, @inbPurchaseOrderID = @inbPurchaseOrderID OUTPUT
	, @bitNewFlag = @bitNewFlag
	, @vchItemOptions = @vchCleanOptions;

RETURN @intReturnValue;
GO
