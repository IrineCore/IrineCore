USE [dnworld]
GO
/****** Object:  Table [dbo].[WishProducts]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[WishProducts](
	[WishProductID] [int] IDENTITY(1,1) NOT NULL,
	[MailID] [bigint] NOT NULL,
	[ProductID] [int] NOT NULL,
	[PurchaseOrderID] [bigint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_WishProducts_MailID]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_WishProducts_MailID] ON [dbo].[WishProducts]
(
	[MailID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_WishProducts_WishProductID]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[WishProducts] ADD  CONSTRAINT [PK_WishProducts_WishProductID] PRIMARY KEY NONCLUSTERED 
(
	[WishProductID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
ALTER TABLE [dbo].[WishProducts]  WITH CHECK ADD  CONSTRAINT [FK_Mails_WishProducts_MailID] FOREIGN KEY([MailID])
REFERENCES [dbo].[Mails] ([MailID])
GO
ALTER TABLE [dbo].[WishProducts] CHECK CONSTRAINT [FK_Mails_WishProducts_MailID]
GO
