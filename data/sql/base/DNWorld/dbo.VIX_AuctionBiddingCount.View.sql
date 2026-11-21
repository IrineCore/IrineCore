USE [dnworld]
GO
/****** Object:  View [dbo].[VIX_AuctionBiddingCount]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[VIX_AuctionBiddingCount] AS
SELECT A.AuctionID, MAX(BiddingPrice) AS MaxBiddingPrice, COUNT(*) AS BiddingCount, MAX(BiddingID) AS LastBiddingID
FROM dbo.AuctionBidding A
GROUP BY A.AuctionID
GO
