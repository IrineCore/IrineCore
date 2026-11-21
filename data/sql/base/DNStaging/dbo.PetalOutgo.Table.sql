USE [dnstaging]
GO
/****** Object:  Table [dbo].[PetalOutgo]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PetalOutgo](
	[PetalOutgoID] [int] NOT NULL,
	[PetalIncomeID] [bigint] NOT NULL,
	[PetalOutgoCode] [tinyint] NOT NULL,
	[PurchaseOrderID] [bigint] NULL,
	[PetalAmount] [int] NOT NULL,
	[OutgoDate] [datetime2](0) NOT NULL,
	[PetalOutgoFKey] [bigint] NULL,
 CONSTRAINT [PK_PetalOutgo] PRIMARY KEY CLUSTERED 
(
	[PetalOutgoID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PetalOutgo_OutgoDate]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PetalOutgo_OutgoDate] ON [dbo].[PetalOutgo]
(
	[OutgoDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PetalOutgo_PetalIncomeID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PetalOutgo_PetalIncomeID] ON [dbo].[PetalOutgo]
(
	[PetalIncomeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PetalOutgo_PurchaseOrderID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PetalOutgo_PurchaseOrderID] ON [dbo].[PetalOutgo]
(
	[PurchaseOrderID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
