USE [dnworld]
GO
/****** Object:  Table [dbo].[WishAuctions]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[WishAuctions](
	[AuctionID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
 CONSTRAINT [PK_WishAuctions] PRIMARY KEY CLUSTERED 
(
	[AuctionID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[WishAuctions]  WITH CHECK ADD  CONSTRAINT [FK_Auctions_WishAuctions_AuctionID] FOREIGN KEY([AuctionID])
REFERENCES [dbo].[Auctions] ([AuctionID])
GO
ALTER TABLE [dbo].[WishAuctions] CHECK CONSTRAINT [FK_Auctions_WishAuctions_AuctionID]
GO
