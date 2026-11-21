USE [dnworld]
GO
/****** Object:  Table [dbo].[AuctionItemsHistory]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AuctionItemsHistory](
	[AuctionItemsHistoryID] [int] IDENTITY(1,1) NOT NULL,
	[AuctionItemsHistoryStatusCode] [tinyint] NOT NULL,
	[AuctionItemsHistoryStatusFKey] [int] NOT NULL,
	[Coin] [int] NULL,
	[RegisterDate] [datetime] NOT NULL,
	[ItemSerial] [bigint] NULL,
	[ItemID] [int] NULL,
	[ItemRemainCount] [smallint] NULL,
	[PayMethodCode] [tinyint] NULL,
	[SenderCharacterID] [bigint] NULL,
	[ItemMaterializeCode] [tinyint] NULL,
	[ItemMaterializeFKey] [bigint] NULL,
	[ItemDurability] [smallint] NULL,
	[RandomSeed] [int] NULL,
	[CoolTime] [int] NULL,
	[ItemLevel] [tinyint] NULL,
	[ItemPotential] [tinyint] NULL,
	[SoulBoundFlag] [bit] NULL,
	[SealCount] [tinyint] NULL,
	[ItemOption] [tinyint] NULL,
	[ItemMaterializeDate] [datetime2](0) NULL,
	[ItemLifespan] [int] NULL,
	[EternityFlag] [bit] NULL,
	[ItemExpireDate] [smalldatetime] NULL,
	[OwnerCharacterID] [bigint] NULL,
	[OwnershipStartDate] [datetime2](0) NULL,
	[ExpireCompleteFlag] [bit] NULL,
	[TradeItemFlag] [bit] NULL,
	[AdditiveItemID] [int] NULL,
	[ItemLocationCode] [tinyint] NULL,
	[ItemPotentialMoveCount] [tinyint] SPARSE  NULL,
	[ItemCount] [smallint] NULL,
	[ItemPrice] [bigint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_AuctionItemsHistory_AuctionItemsHistoryStatusFKey_AuctionItemsHistoryStatusCode]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_AuctionItemsHistory_AuctionItemsHistoryStatusFKey_AuctionItemsHistoryStatusCode] ON [dbo].[AuctionItemsHistory]
(
	[AuctionItemsHistoryStatusFKey] ASC,
	[AuctionItemsHistoryStatusCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_AuctionItemsHistory]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[AuctionItemsHistory] ADD  CONSTRAINT [PK_AuctionItemsHistory] PRIMARY KEY NONCLUSTERED 
(
	[AuctionItemsHistoryID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
