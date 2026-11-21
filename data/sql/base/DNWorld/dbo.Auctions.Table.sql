USE [dnworld]
GO
/****** Object:  Table [dbo].[Auctions]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Auctions](
	[AuctionID] [int] IDENTITY(1,1) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CategoryCode] [tinyint] NOT NULL,
	[StartPrice] [bigint] NOT NULL,
	[Commission] [bigint] NOT NULL,
	[BuynowPrice] [bigint] NOT NULL,
	[AuctionStatusCode] [tinyint] NOT NULL,
	[BiddingEndDate] [smalldatetime] NOT NULL,
	[RegistrationDate] [smalldatetime] NOT NULL,
	[UpdateDate] [smalldatetime] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[ItemLevel] [tinyint] NULL,
	[ItemCount] [smallint] NULL,
	[ItemOption] [tinyint] NULL,
	[ItemPotential] [tinyint] NULL,
	[SealCount] [tinyint] NULL,
	[ItemDurability] [smallint] NULL,
	[ItemPotentialMoveCount] [tinyint] NULL,
	[RandomSeed] [int] NULL,
	[CoinCalculateFlag] [bit] NOT NULL,
	[ItemCalculateFlag] [bit] NOT NULL,
 CONSTRAINT [PK_Autions] PRIMARY KEY CLUSTERED 
(
	[AuctionID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_Auctions_BiddingEndDate_BuynowPrice]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Auctions_BiddingEndDate_BuynowPrice] ON [dbo].[Auctions]
(
	[BiddingEndDate] ASC,
	[BuynowPrice] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_Auctions_CategoryCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Auctions_CategoryCode] ON [dbo].[Auctions]
(
	[CategoryCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_Auctions_CharacterID_AuctionStatusCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Auctions_CharacterID_AuctionStatusCode] ON [dbo].[Auctions]
(
	[CharacterID] ASC,
	[AuctionStatusCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Auctions_ItemID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_Auctions_ItemID] ON [dbo].[Auctions]
(
	[ItemID] ASC
)
INCLUDE([AuctionStatusCode],[BiddingEndDate]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[Auctions] ADD  CONSTRAINT [DF_Auctions_CoinCaculateFlag]  DEFAULT ((0)) FOR [CoinCalculateFlag]
GO
ALTER TABLE [dbo].[Auctions] ADD  CONSTRAINT [DF_Auctions_ItemCalculateFlag]  DEFAULT ((0)) FOR [ItemCalculateFlag]
GO
