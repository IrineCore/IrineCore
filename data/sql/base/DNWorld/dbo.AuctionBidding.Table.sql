USE [dnworld]
GO
/****** Object:  Table [dbo].[AuctionBidding]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AuctionBidding](
	[BiddingID] [int] IDENTITY(1,1) NOT NULL,
	[AuctionID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[BiddingPrice] [bigint] NOT NULL,
	[BiddingStatusCode] [tinyint] NOT NULL,
	[BiddingDate] [smalldatetime] NOT NULL,
 CONSTRAINT [PK_AuctionBidding] PRIMARY KEY CLUSTERED 
(
	[BiddingID] ASC,
	[AuctionID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NNI_AuctionBidding_CharacterID_BiddingStatusCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_AuctionBidding_CharacterID_BiddingStatusCode] ON [dbo].[AuctionBidding]
(
	[CharacterID] ASC,
	[BiddingStatusCode] ASC
)
INCLUDE([BiddingPrice]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[AuctionBidding]  WITH CHECK ADD  CONSTRAINT [FK_Auctions_AuctionBidding_AuctionID] FOREIGN KEY([AuctionID])
REFERENCES [dbo].[Auctions] ([AuctionID])
GO
ALTER TABLE [dbo].[AuctionBidding] CHECK CONSTRAINT [FK_Auctions_AuctionBidding_AuctionID]
GO
