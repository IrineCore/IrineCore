USE [dnworld]
GO
/****** Object:  Table [dbo].[AuctionCancelReason]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AuctionCancelReason](
	[AuctionID] [int] NOT NULL,
	[CancelReason] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CancelDate] [datetime2](7) NOT NULL,
 CONSTRAINT [PK_AuctionCancelReason] PRIMARY KEY CLUSTERED 
(
	[AuctionID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[AuctionCancelReason]  WITH CHECK ADD  CONSTRAINT [FK_Auctions_AuctionCancelReason_AuctionID] FOREIGN KEY([AuctionID])
REFERENCES [dbo].[Auctions] ([AuctionID])
GO
ALTER TABLE [dbo].[AuctionCancelReason] CHECK CONSTRAINT [FK_Auctions_AuctionCancelReason_AuctionID]
GO
