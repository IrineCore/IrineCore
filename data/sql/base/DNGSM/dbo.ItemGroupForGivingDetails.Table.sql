USE [dngsm]
GO
/****** Object:  Table [dbo].[ItemGroupForGivingDetails]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemGroupForGivingDetails](
	[ItemGroupID] [int] NOT NULL,
	[ItemGroupDetailID] [int] IDENTITY(1,1) NOT NULL,
	[ItemID] [int] NULL,
	[Quantity] [int] NULL,
	[ProductID] [int] NULL,
	[AppellationID] [int] NULL,
 CONSTRAINT [PK_ItemGroupForGivingDetails] PRIMARY KEY CLUSTERED 
(
	[ItemGroupID] ASC,
	[ItemGroupDetailID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[ItemGroupForGivingDetails]  WITH CHECK ADD  CONSTRAINT [FK_ItemGroupsForGiving_ItemGroupForGivingDetails_ItemGroupID] FOREIGN KEY([ItemGroupID])
REFERENCES [dbo].[ItemGroupsForGiving] ([ItemGroupID])
GO
ALTER TABLE [dbo].[ItemGroupForGivingDetails] CHECK CONSTRAINT [FK_ItemGroupsForGiving_ItemGroupForGivingDetails_ItemGroupID]
GO
